import os
import platform
import subprocess
from threading import Thread

import nanomsg

from cetech.consoleapi import ConsoleAPI


class ReadyLock(Thread):
    def __init__(self, url, on_ready=None):
        self.url = url if isinstance(url, bytes) else url.encode()
        self.on_ready = on_ready

        self.ready = False

        self.socket = nanomsg.Socket(nanomsg.PULL)
        self.socket.bind(self.url)

        super().__init__()

    def wait_for_ready(self):
        while not self.ready:
            pass

    def run(self):
        # TODO: quit freeze. rewrite to non blocking with exit value
        recv = self.socket.recv()
        self.ready = True
        if self.on_ready is not None:
            self.on_ready()

        self.socket.close()


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
        self.push_url = "ws://*:%s" % 4446
        self.pub_url = "ws://*:%s" % 4447
        self.ready = False
        self._console_api = None

    @property
    def console_api(self):
        return self._console_api

    def kill(self, dump=False):
        if dump:
            self._dump()

        self.process.kill()

    def run_release(self, build_dir):
        args = [
            "-build %s" % build_dir,
        ]

        return self._run(self.BUILD_RELEASE, args, lock=False)

    def run_develop(self, build_dir, source_dir, compile_=False, continue_=False, wait=False, daemon=False,
                    wid=None, core_dir=None, port=None, bootscript=None, bootpkg=None, protocol='ws', check=False,
                    lock=True):
        args = [
            "-build %s" % build_dir,
            "-src %s" % source_dir
        ]

        if compile_:
            args.append("-compile")
            args.append("-external %s" % self.external_dir)

        if continue_:
            args.append("-continue")

        if wait:
            args.append("-wait")

        if bootscript:
            args.append("-core.boot_script %s" % bootscript)

        if bootpkg:
            args.append("-core.boot_pkg %s" % bootpkg)

        if daemon:
            args.append("-daemon")

        if port:
            self.rpc_url = "%s://localhost:%s" % (protocol, port)
            self.log_url = "%s://localhost:%s" % (protocol, port + 1)
            self.push_url = "%s://*:%s" % (protocol, port + 2)
            self.pub_url = "%s://*:%s" % (protocol, port + 3)

            args.append("-develop.rpc.port %s" % port)
            args.append("-develop.rpc.addr %s://*" % (protocol))

            args.append("-develop.log.port %s" % (port + 1))
            args.append("-develop.log.addr %s://*" % (protocol))

            args.append("-develop.pub.port %s" % (port + 3))
            args.append("-develop.pub.addr %s://*" % (protocol))

        # TODO bug #114 workaround. Disable create sub engine...
        if wid and platform.system().lower() != 'darwin':
            args.append("-wid %s" % int(wid))

        if core_dir:
            args.append("-core %s" % core_dir)
        else:
            args.append("-core ../core")  # TODO ?

        self._run(self.BUILD_DEVELOP, args, check=check, lock=lock)

    def _run(self, build_type, args, lock, check=False):
        if lock:
            def _on_ready():
                self.ready = True

            self.rl = ReadyLock(self.push_url, on_ready=_on_ready)
            self.rl.start()

        if build_type == self.BUILD_DEVELOP:
            self._console_api = ConsoleAPI(self.rpc_url)
            args.append("-develop.push.addr %s" % self.push_url.replace("*", "localhost"))

        cmd = "%s %s" % (self._get_executable_path(build_type), ' '.join(args))
        print(cmd)

        self.process = subprocess.Popen(cmd.split(' '), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        if build_type == self.BUILD_DEVELOP:
            self._console_api.connect()

            if check:
                self.process.wait()

    def _dump(self):
        p = self.process

        print("=== Process: %s ===" % self.name)
        # print(''.join([x.decode() for x in iter(p.stdout.readline, b'')]))
        print("===================%s\n" % ('=' * len(self.name)))

    def _get_lib_path(self):
        _platform = platform.system().lower()
        bin_dir = "%s%s" % (_platform if _platform != 'darwin' else 'osx', platform.architecture()[0][0:2])
        return os.path.join(self.bin_dir, bin_dir)

    def _get_executable_path(self, build_type):
        engine_bin_path = os.path.join(self._get_lib_path(), self._ENGINE_BIN[build_type])
        return engine_bin_path
