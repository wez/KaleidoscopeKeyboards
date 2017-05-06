#!/usr/bin/env python
import os
import sys
import subprocess
import argparse
import re
import shutil
import shlex
from glob import glob
import time
from pprint import pprint
from urlparse import urlparse

parser = argparse.ArgumentParser(description='''
    build keyboard firmware
    ''')

parser.add_argument('keyboard', help='which keyboard to build')
parser.add_argument('--clean', help='remove build products', action='store_true')
parser.add_argument('--flash', help='flash the device', action='store_true')
parser.add_argument('--verbose', help='verbose build', action='store_true')
parser.add_argument('--sync', help='perform submodule sync', action='store_true')
parser.add_argument('--port', help='override port for flashing')

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

def resolve_pref(prefs, key):
    ''' Evaluate a pref value, expanding interpolated keys '''
    value = prefs[key]
    r = re.compile('\\{([a-zA-Z0-9_._]+)\\}')

    path = []
    for ele in key.split('.'):
        if path:
            path.append('%s.%s' % (path[-1], ele))
        else:
            path.append(ele)

    while True:
        before = value

        m = r.search(value)
        if not m:
            return value

        key = m.group(1)
        if key not in prefs:
            # search configuration path
            key = None
            for ele in path:
                k = '%s.%s' % (ele, m.group(1))
                if k in prefs:
                    key = k
                    break
            if not key:
                raise Exception('could not resolve prefs from %s' % value)

        value = value.replace(m.group(0), prefs[key])

        if value == before:
            break
    return value


    while '{' in value:
        # this is a dumb loop; could be made faster by parsing the
        # string and looking up the keys, rather than iterating
        # over all of them.
        before = value
        for k, v in prefs.items():
            s = '{%s}' % k
            value = value.replace(s, v)

        if value == before:
            # stop if we didn't change anything
            break

    return value


arduino = Arduino()
#pprint(arduino.prefs)

class keyboard(object):
    def __init__(self,
                 name=None,
                 fqbn=None,
                 vid=None,
                 pid=None,
                 product=None,
                 libs=None,
                 manufacturer=None):
        self.name = name
        self.fqbn = fqbn
        self.vid = vid
        self.pid = pid
        self.product = product
        self.manufacturer = manufacturer
        self.libs = libs or []

    def set_dir(self, path):
        self.dir = path

    def _builder(self, mode, prefs=None, prefs_extend=None):
        if prefs_extend:
            base_prefs = self.parse_prefs()

            for k, v in prefs_extend.items():
                if k in base_prefs:
                    prefs[k] = '%s %s' % (base_prefs[k], v)
                else:
                    prefs[k] = v

        home = os.environ['HOME']
        cmd = [
            os.path.join(arduino.prefs['runtime.ide.path'], 'arduino-builder'),
            mode,
            '-hardware', os.path.join(arduino.prefs['runtime.ide.path'], 'hardware'),
            '-hardware', '%s/Library/Arduino15/packages' % home,
            '-tools', os.path.join(arduino.prefs['runtime.ide.path'], 'tools-builder'),
            '-tools', os.path.join(arduino.prefs['runtime.ide.path'], 'hardware', 'tools', 'avr'),
            '-tools', '%s/Library/Arduino15/packages' % home,
            '-fqbn', self.fqbn,
            '-built-in-libraries', os.path.join(arduino.prefs['runtime.ide.path'], 'libraries'),
            '-libraries', os.path.realpath('libs'),
            '-libraries', '%s/externals' % self.dir,
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

    def get_libs(self):
        ''' ensure that we have each of the listed libs available '''

        # externals is the shared directory holding the libs
        externals = os.path.realpath('externals')
        if not os.path.isdir(externals):
            os.makedirs(externals)

        # local externals is the local "mirror" of the shared directory.
        # it is maintained separately so that we can trap missing deps
        # in one of the projects
        local_externals = os.path.join(self.dir, 'externals')
        if not os.path.isdir(local_externals):
            os.makedirs(local_externals)

        # to be able to garbage collect removed deps, scan the local dir
        # and remember the previous set of deps.  We'll remove any of
        # these that aren't referenced below.
        to_remove = set(os.listdir(local_externals))

        for lib in self.libs:
            url = urlparse(lib)
            local_dir = os.path.join(externals, os.path.basename(url.path))
            if not os.path.isdir(local_dir):
                subprocess.check_call(['git', 'clone', '--depth', '1', lib, local_dir])
            elif args.sync:
                print('Updating %s' % local_dir)
                subprocess.check_call(['git', 'fetch', 'origin'], cwd=local_dir)
                subprocess.check_call(['git', 'rebase', 'origin', 'master'], cwd=local_dir)

            to_remove.discard(os.path.basename(url.path))
            local_link = os.path.join(local_externals, os.path.basename(url.path))
            if os.path.islink(local_link):
                if os.readlink(local_link) == local_dir:
                    continue
                os.unlink(local_link)
            elif os.path.exists(local_link):
                print(('WARNING: %s exists and is not a symlink '
                       'managed by the libs directive') % local_link)
                continue

            os.symlink(local_dir, local_link)

        for lib in to_remove:
            os.unlink(os.path.join(local_externals, lib))


    def build(self):
        build_path = self.make_dirs()

        self.get_libs()

        prefs = self.parse_prefs()

        if args.verbose:
            print('Builder prefs reported as:')
            pprint(prefs)

        cmd = self._builder(
            '-compile',
            prefs={
                'build.pid': self.pid,
                'build.vid': self.vid,
                'build.usb_manufacturer': '"%s"' % self.manufacturer,
                'build.usb_product': '"%s"' % self.product,

                # fixup what looks like a bogus config for teensy
                'recipe.ar.pattern':
                    prefs['recipe.ar.pattern'].replace(
                        '{build.path}/core/{archive_file}',
                        '{archive_file_path}'),
            },
            prefs_extend={
                'compiler.cpp.extra_flags': ' '.join([
                    # '-std=c++11', adafruit libs are not C++11 clean
                    '-Woverloaded-virtual',
                    '-Wno-unused-parameter',
                    '-Wno-unused-variable',
                    '-Wno-ignored-qualifiers',
                    '\'-DKBD_MANUFACTURER="%s"\'' % self.manufacturer,
                    '\'-DKBD_PRODUCT="%s"\'' % self.product,
                    ]),
                # AVR and Feather pick up flags from here
                'build.extra_flags': ' '.join([
                    '-DKALEIDOSCOPE_HARDWARE_H="%s-hardware.h"' % self.name,
                    ]),
                # The teensy core picks up flags from here
                'build.flags.cpp': ' '.join([
                    '-DKALEIDOSCOPE_HARDWARE_H="%s-hardware.h"' % self.name,
                    ]),
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

        prefs = self.parse_prefs()
        p = dict(arduino.prefs)
        p.update(prefs)
        tool = prefs['upload.tool']

        params = {
            'arduino:avrdude': {
                'key': 'tools.avrdude',
                'port_glob': '/dev/cu.*',
            },
            'nrfutil': {
                'key': 'tools.nrfutil',
                'port_glob': '/dev/cu.SLAB*',
            },
            'teensyloader': {
                'key': 'tools.teensyloader',
                'port_glob': None,
            },
        }

        if tool in params:
            self._flash(params[tool], p)
        else:
            pprint(p)
            print("I don't know how to flash using %s" % tool)

    def _flash(self, params, p):
        key = params['key']

        p['upload.verbose'] = '{%s.upload.params.verbose}' % key \
                               if args.verbose \
                               else '{%s.upload.params.quiet}' % key

        p['build.path'] = self.output_dir_name()
        p['build.project_name'] = '%s.ino' % self.name

        if args.verbose:
            print('Dict for flashing')
            pprint(p)

        pref_serial_port = resolve_pref(p, 'serial.port')

        while True:
            device = None

            if args.port:
                device = args.port
            elif params['port_glob']:
                devices = glob(params['port_glob'])
                if len(devices) == 1:
                    device = devices[0]
                elif len(devices) > 0:
                    print('Ambiguous set of devices; please reset the appropriate one manually!')
                    pprint(devices)
                    device = pref_serial_port

            if device:
                p['serial.port'] = device

            if device:
                # try to persuade the device to jump to the bootloader
                # (this doesn't seem to work with all hardware)
                subprocess.call(['stty', '-f', device, '1200'])

            cmd = resolve_pref(p, '%s.upload.pattern' % key)
            cmd = shlex.split(cmd)
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

if args.clean:
    kbd.clean()
elif args.flash:
    sys.exit(0 if kbd.flash() else 1)
else:
    sys.exit(0 if kbd.build() else 1)
