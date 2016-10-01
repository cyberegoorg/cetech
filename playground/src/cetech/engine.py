import os
import platform
import subprocess
from time import sleep

import shutil
from PyQt5.QtCore import QProcess, QProcessEnvironment

from cetech.consoleapi import ConsoleAPI


class EngineInstance(object):
    BUILD_DEVELOP = 'Develop'
    BUILD_RELEASE = 'Release'

    _BIN_PLATFORM = {
        'windows': 'windows',
        'linux': 'linux'
    }

    _ENGINE_BIN = {
        BUILD_DEVELOP: 'cetech_develop',
        BUILD_RELEASE: 'cetech'
    }

    def __init__(self, name, bin_dir, external_dir):
        self.name = name
        self.process = None
        self.bin_dir = bin_dir
        self.external_dir = external_dir
        self.rpc_url = "ws://localhost:%s" % 4444
        self.log_url = "ws://localhost:%s" % 4445

    def create_console_api(self):
        return ConsoleAPI(self.rpc_url)

    def get_lib_path(self):
        _platform = platform.system().lower()
        bin_dir = "%s%s" % (_platform if _platform != 'darwin' else 'osx', platform.architecture()[0][0:2])
        return os.path.join(self.bin_dir, bin_dir)

    def get_executable_path(self, build_type):
        engine_bin_path = os.path.join(self.get_lib_path(), self._ENGINE_BIN[build_type])
        return engine_bin_path

    def run_cetech_release(self, build_dir):
        args = [
            "-b %s" % build_dir,
        ]

        return self._run(self.BUILD_RELEASE, args)

    def run_develop(self, build_dir, source_dir, compile_=False, continue_=False, wait=False, daemon=False,
                    wid=None,
                    core_dir=None, port=None, bootscript=None, protocol='ws'):
        args = [
            "-s .build %s" % build_dir,
            "-s .src %s" % source_dir
        ]

        if compile_:
            args.append("-s .compile 1")
            args.append("-s .external %s" % self.external_dir)

        if continue_:
            args.append("-s .continue 1")

        if wait:
            args.append("-s .wait 1")

        if bootscript:
            args.append("-s core.boot_scrpt %s" % bootscript)

        if daemon:
            args.append("-s .daemon 1")

        if port:
            self.rpc_url = "%s://localhost:%s" % (protocol, port)
            self.log_url = "%s://localhost:%s" % (protocol, port + 1)

            args.append("-s develop.rpc.port %s" % port)
            args.append("-s develop.rpc.addr %s://*" % (protocol))

            args.append("-s develop.log.port %s" % (port + 1))
            args.append("-s develop.log.addr %s://*" % (protocol))

        # TODO bug #114 workaround. Disable create sub engine...
        if wid and platform.system().lower() != 'darwin':
            args.append("-s .wid %s" % int(wid))

        if core_dir:
            args.append("-s .core %s" % core_dir)
        else:
            args.append("-s .core ../core")  # TODO ?

        self._run(self.BUILD_DEVELOP, args)

    def _run(self, build_type, args):
        cmd = "%s %s" % (self.get_executable_path(build_type), ' '.join(args))
        print(cmd)

        self.process = subprocess.Popen(cmd.split(' '), stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    def kill(self, dump=False):
        if dump:
            self._dump()

        self.process.terminate()

    def _dump(self):
        p = self.process

        # out, err = bytearray(p.readAllStandardOutput()).decode(), bytearray(p.readAllStandardError()).decode()

        print("=== Process: %s ===" % self.name)
        print(''.join([x.decode() for x in iter(p.stdout.readline, b'')]))
        print("===================%s\n" % ('=' * len(self.name)))
