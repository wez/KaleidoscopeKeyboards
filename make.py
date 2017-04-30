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

    def _builder(self, mode):
        def quote(name):
            return name.replace(" ", "_")

        return [
            '/Applications/Arduino.app/Contents/Java/arduino-builder',
            mode,
            '-hardware', '/Applications/Arduino.app/Contents/Java/hardware',
            '-hardware', '/Users/wez/Library/Arduino15/packages',
            '-tools', '/Applications/Arduino.app/Contents/Java/hardware/tools',
            '-tools', '/Applications/Arduino.app/Contents/Java/tools-builder',
            '-fqbn', self.fqbn,
            '-libraries', '/Users/wez/Documents/Arduino/libraries',
            '-libraries', '/Users/wez/Library/Arduino15/packages/..',
            '-libraries', '/Users/wez/src/kaleidoscope/WezsKeyboards/keyboardio/Kaleidoscope',
            '-libraries', '/Users/wez/src/kaleidoscope/WezsKeyboards/iota/../keyboardio/Arduino-Boards/libraries',
            '-hardware', '/Users/wez/src/kaleidoscope/WezsKeyboards/iota/../keyboardio/Arduino-Boards',
            '-libraries', '/Users/wez/src/kaleidoscope/WezsKeyboards/iota',
            '-prefs', 'build.extra_flags=-DKALEIDOSCOPE_HARDWARE_H="{name}-hardware.h" -DUSB_VID={vid} -DUSB_PID={pid} -DUSB_PRODUCT="{product}" -DUSB_MANUFACTURER="{manufacturer}"'.format(name=self.name,
                                             vid=self.vid,
                                             pid=self.pid,
                                             product=quote(self.product),
                                             manufacturer=quote(self.manufacturer)),
            '-build-path', self.make_dirs(),
            '-ide-version', '100607',
            '-warnings', 'all',
            '-verbose',
            '-prefs', 'compiler.cpp.extra_flags=-std=c++11 -Woverloaded-virtual -Wno-unused-parameter -Wno-unused-variable -Wno-ignored-qualifiers',
            '%s/keymap/%s.ino' % (self.dir, self.name)
            ]

    def output_dir_name(self):
        return os.path.realpath(os.path.join('outputs', self.name))

    def make_dirs(self):
        path = self.output_dir_name()
        if not os.path.isdir(path):
            os.makedirs(path)
        return path

    def build(self):
        build_path = self.make_dirs()
        cmd = self._builder('-compile')
        pprint(cmd)
        result = subprocess.call(cmd)
        pprint(result)

    def clean(self):
        path = self.output_dir_name()
        if os.path.isdir(path):
            shutil.rmtree(path)
        print('Cleaned %s' % path)

    def flash(self):
        self.build()

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


args = parser.parse_args()

def load_keyboard(name):
    filename = os.path.join(name, 'info.py')
    with open(filename, 'r') as f:
        code = compile(f.read(), filename, 'eval')
        return eval(code)

kbd = load_keyboard(args.keyboard)
kbd.set_dir(os.path.realpath(args.keyboard))

if args.clean:
    kbd.clean()
elif args.flash:
    kbd.flash()
else:
    kbd.build()
