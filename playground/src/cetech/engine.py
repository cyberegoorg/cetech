import logging
import os
import platform
import subprocess
from threading import Thread

import nanomsg

from cetech.consoleapi import ConsoleAPI


class _ReadyLock(Thread):
    def __init__(self, url, on_ready=None):
        self.url = url if isinstance(url, bytes) else url.encode()
        self.on_ready = on_ready

        logging.debug('URL %s', self.url)

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


class Instance(object):
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

    def __init__(self, name, bin_dir, external_dir, logger=logging):
        self.name = name
        self.process = None
        self.bin_dir = bin_dir
        self.external_dir = external_dir
        self.ready = False
        self.logger = logger
        self._console_api = None

        self.rpc_url = "ws://localhost:%s" % 4444
        self.log_url = "ws://localhost:%s" % 4445
        self.push_url = "ws://*:%s" % 4446
        self.pub_url = "ws://*:%s" % 4447

    @property
    def console_api(self):
        return self._console_api

    def kill(self, dump=False):
        if dump:
            self._dump()

        self.logger.debug("Killing")
        self._console_api.close()
        self.process.kill()
        self.process.wait()
        self.logger.debug("Killed")

    def run_release(self, build_dir):
        args = [
            ("build", build_dir)
        ]

        return self._run_raw(self.BUILD_RELEASE, args, lock=False)

    def _args_to_str(self, args):
        return ''.join([" -%s %s" % (opt, val if val is not None else '1') for opt, val in args])

    def run_develop(self, build_dir, source_dir, compile_=False, continue_=False, wait=False, daemon=False,
                    wid=None, core_dir=None, bootscript=None, bootpkg=None, protocol='ws', check=False,
                    lock=True, rpc_addr=None, log_addr=None, pub_addr=None, push_addr=None):
        args = [
            ("build", build_dir),
            ("src", source_dir),
        ]

        if compile_:
            args.append(("compile", None))
            args.append(("external", self.external_dir))

        if continue_:
            args.append(("continue", None))

        if wait:
            args.append(("wait", None))

        if bootscript:
            args.append(("core.boot_script", bootscript))

        if bootpkg:
            args.append(("core.boot_pkg", bootpkg))

        if daemon:
            args.append(("daemon", None))

        if rpc_addr is None:
            self.rpc_url = "%s://localhost:%s" % (protocol, port)
            args.append(("develop.rpc.addr", "%s://*:%s" % (protocol, port)))
        else:
            args.append(("develop.rpc.addr", rpc_addr))
            self.rpc_url = rpc_addr

        if log_addr is None:
            self.log_url = "%s://localhost:%s" % (protocol, port + 1)
            args.append(("develop.log.addr", "%s://*:%s" % (protocol, port + 1)))
        else:
            args.append(("develop.log.addr", log_addr))
            self.log_url = log_addr

        if pub_addr is None:
            self.pub_url = "%s://*:%s" % (protocol, port + 3)
            args.append(("develop.pub.addr", "%s://*:%s" % (protocol, port + 3)))
        else:
            args.append(("develop.pub.addr", pub_addr))
            self.pub_url = pub_addr

        if push_addr is None:
            self.push_url = "%s://*:%s" % (protocol, port + 2)
            args.append(("develop.push.addr", "%s://*:%s" % (protocol, port + 3)))
        else:
            args.append(("develop.push.addr", push_addr))
            self.push_url = push_addr

        # TODO bug #114 workaround. Disable create sub engine...
        if wid and platform.system().lower() != 'darwin':
            args.append(("wid", int(wid)))

        if core_dir:
            args.append(("core", core_dir))

        else:
            # TODO: absolute path
            args.append(("core", "../core"))

        self._run_raw(self.BUILD_DEVELOP, args, check=check, lock=lock)

    def _run_raw(self, build_type, args, lock, check=False):
        if lock:
            def _on_ready():
                self.ready = True

            self.rl = _ReadyLock(self.push_url, on_ready=_on_ready)
            self.rl.start()

        args_str = self._args_to_str(args)

        cmd = "%s %s" % (self._get_executable_path(build_type), args_str)

        self.logger.debug("Run %s %s", self._ENGINE_BIN[build_type], args_str)

        self.process = subprocess.Popen(cmd.split(' '), stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        if build_type == self.BUILD_DEVELOP:
            self._console_api = ConsoleAPI(self.rpc_url)
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
