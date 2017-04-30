#!/usr/bin/env python
import os
import sys
import subprocess
import argparse
import shutil
from glob import glob
import time
from pprint import pprint

parser = argparse.ArgumentParser(description='''
    build keyboard firmware
    ''')

parser.add_argument('keyboard', help='which keyboard to build')
parser.add_argument('--clean', help='remove build products', action='store_true')
parser.add_argument('--flash', help='flash the device', action='store_true')
parser.add_argument('--verbose', help='verbose build', action='store_true')
parser.add_argument('--sync', help='perform submodule sync', action='store_true')

args = parser.parse_args()

class Arduino(object):
    def __init__(self):
        self.arduino = self.find_arduino()
        self.prefs = self.load_prefs()

    def find_arduino(self):
        candidates = ['/Applications/Arduino.app/Contents/MacOS/Arduino', 'arduino', 'arduino.exe']

        path = os.environ['PATH'].split(os.pathsep)

        for c in candidates:
            if os.path.isfile(c):
                return c
            if not os.path.isabs(c):
                for p in path:
                    f = os.path.join(p, c)
                    if os.path.isfile(f):
                        return f
        return None

    def load_prefs(self):
        prefs = os.path.join('outputs', 'prefs.txt')
        if not os.path.isfile(prefs):
            if not os.path.isdir('outputs'):
                os.mkdir('outputs')

            with open(prefs, 'w') as f:
                subprocess.check_call([self.arduino, '--get-pref'], stdout=f)

        result = {}
        with open(prefs, 'r') as f:
            for line in f:
                cols = line.rstrip('\n').split("=", 1)
                if len(cols) == 2:
                    result[cols[0]] = cols[1]
        return result

arduino = Arduino()
#pprint(arduino.prefs)

AVRDUDE = '/Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/avrdude'

class keyboard(object):
    def __init__(self,
                 name=None,
                 fqbn=None,
                 vid=None,
                 pid=None,
                 product=None,
                 mcu=None,
                 manufacturer=None):
        self.name = name
        self.fqbn = fqbn
        self.mcu = mcu
        self.vid = vid
        self.pid = pid
        self.product = product
        self.manufacturer = manufacturer

    def set_dir(self, path):
        self.dir = path

    def _builder(self, mode, prefs=None, prefs_extend=None):
        boards = os.path.join('keyboardio', 'Arduino-Boards')
        libs = os.path.join(boards, 'libraries')

        if prefs_extend:
            base_prefs = self.parse_prefs()

            for k, v in prefs_extend.items():
                if k in base_prefs:
                    prefs[k] = '%s %s' % (base_prefs[k], v)
                else:
                    prefs[k] = v

        cmd = [
            os.path.join(arduino.prefs['runtime.ide.path'], 'arduino-builder'),
            mode,
            '-hardware', '/Applications/Arduino.app/Contents/Java/hardware',
            '-hardware', '/Users/wez/Library/Arduino15/packages',
            '-tools', '/Applications/Arduino.app/Contents/Java/tools-builder',
            '-tools', '/Applications/Arduino.app/Contents/Java/hardware/tools/avr',
            '-tools', '/Users/wez/Library/Arduino15/packages',
            '-fqbn', self.fqbn,
            '-built-in-libraries', '/Applications/Arduino.app/Contents/Java/libraries',
            '-libraries', '/Users/wez/Documents/Arduino/libraries',
            '-libraries', libs,
            #'-hardware', boards,
            '-libraries', self.dir,
            '-build-path', self.make_dirs(),
            '-ide-version', arduino.prefs['runtime.ide.version'],
            '-warnings', 'all',
            '-verbose' if args.verbose else '-quiet',
            ]

        if prefs:
            for k, v in prefs.items():
                cmd.append('-prefs=%s=%s' % (k, v))

        for k, v in arduino.prefs.items():
            if k.startswith('runtime.tools.'):
                cmd.append('-prefs=%s=%s' % (k, v))

        cmd += [
            '%s/keymap/%s.ino' % (self.dir, self.name)
            ]

        return cmd

    def output_dir_name(self):
        return os.path.realpath(os.path.join('outputs', self.name))

    def make_dirs(self):
        path = self.output_dir_name()
        if not os.path.isdir(path):
            os.makedirs(path)
        return path

    def parse_prefs(self):
        cmd = self._builder('-dump-prefs')
        out = subprocess.check_output(cmd)
        prefs = {}
        for line in out.splitlines():
            cols = line.split("=", 1)
            if len(cols) == 2:
                prefs[cols[0]] = cols[1]
            else:
                print('malformed line: %s' % line)
        return prefs

    def build(self):
        build_path = self.make_dirs()
        cmd = self._builder('-compile', prefs={
            'build.pid': self.pid,
            'build.vid': self.vid,
            'build.usb_manufacturer': '"%s"' % self.manufacturer,
            'build.usb_product': '"%s"' % self.product,
            },
            prefs_extend={
            'compiler.cpp.extra_flags': ' '.join([
             #   '-std=c++11', adafruit libs are not C++11 clean
                '-Woverloaded-virtual',
                '-Wno-unused-parameter',
                '-Wno-unused-variable',
                '-Wno-ignored-qualifiers',
                ]),
            'build.extra_flags': ' '.join([
                '-DKALEIDOSCOPE_HARDWARE_H="%s-hardware.h"' % self.name,
                ])
            })
        if args.verbose:
            print('Building using:')
            pprint(cmd)
        result = subprocess.call(cmd)
        if result != 0:
            print('Build FAILED!')
            return False
        print('Built OK!')
        return True

    def clean(self):
        path = self.output_dir_name()
        if os.path.isdir(path):
            shutil.rmtree(path)
        print('Cleaned %s' % path)

    def flash(self):
        if not self.build():
            return False

        while True:
            devices = glob('/dev/cu.usbmodem14*')
            device = None
            if len(devices) == 1:
                device = devices[0]
                subprocess.call(['stty', '-f', devices[0], '1200'])
            elif len(devices) > 0:
                print('Ambiguous set of devices; please reset the appropriate one manually!')
                pprint(devices)

            hexpath = os.path.join(self.output_dir_name(), '%s.ino.hex' % self.name)
                
            cmd = [AVRDUDE, '-v', 
                    '-C/Applications/Arduino.app/Contents/Java/hardware/tools/avr/etc/avrdude.conf',
                    '-p%s' % self.mcu, '-cavr109', '-b57600', '-D', "-Uflash:w:%s:i" % hexpath]
            if device:
                cmd += ['-P', device]
            pprint(cmd)

            time.sleep(1)
            result = subprocess.call(cmd)
            if result == 0:
                break
            print('(Failed, will retry)')


def load_keyboard(name):
    filename = os.path.join(name, 'info.py')
    with open(filename, 'r') as f:
        code = compile(f.read(), filename, 'eval')
        return eval(code)

kbd = load_keyboard(args.keyboard)
kbd.set_dir(os.path.realpath(args.keyboard))

if args.sync:
    subprocess.check_call(['git', 'submodule', 'sync', '--recursive'])
    subprocess.check_call(['git', 'submodule', 'update', '--init', '--recursive'])

if args.clean:
    kbd.clean()
elif args.flash:
    sys.exit(0 if kbd.flash() else 1)
else:
    sys.exit(0 if kbd.build() else 1)
