import os
import platform

from PyQt5.QtCore import QDir, QProcess, QProcessEnvironment
from PyQt5.QtWidgets import QFileDialog, QMessageBox


def validate_project(project_dir):
    selected_dir = QDir(project_dir)

    if not selected_dir.exists():
        QMessageBox.critical(None, 'Dir does not  exist', 'Dir does not  exist',
                             QMessageBox.Yes, QMessageBox.Yes)
        return False

    if not selected_dir.exists('src/global.config'):
        QMessageBox.critical(None, 'Project validation error', 'Project dir does not contain src/global.config',
                             QMessageBox.Yes, QMessageBox.Yes)
        return False

    return True


class EngineInstance(object):
    BUILD_DEVELOP = 'Develop'
    BUILD_RELEASE = 'Release'

    _BIN_PLATFORM = {
        'windows': 'Windows',
        'linux': 'Linux'
    }

    _ENGINE_BIN = {
        BUILD_DEVELOP: 'CETechDevelop.exe',
        BUILD_RELEASE: 'CETech.exe'
    }

    _BUILD_DIR = {
        BUILD_DEVELOP: 'Debug',
        BUILD_RELEASE: 'Release'
    }

    def __init__(self, name):
        self.name = name

        self.process = None

    def get_lib_path(self, build_type):
        _platform = platform.system().lower()

        engine_bin_path = os.path.join('..', 'sources', 'CETech', 'bin', self._BIN_PLATFORM[_platform], 'AnyCPU',
                                       'Debug')
        return engine_bin_path

    def get_executable_path(self, build_type):
        engine_bin_path = os.path.join(self.get_lib_path(build_type), self._ENGINE_BIN[build_type])
        return engine_bin_path

    def run_cetech_release(self, build_dir):
        args = [
            "-b %s" % build_dir,
        ]

        return self._run_cetech(self.BUILD_RELEASE, args)

    def run_cetech_develop(self, build_dir, source_dir, compile_=False, continue_=False, wait=False, daemon=False,
                           wid=None,
                           core_dir=None, port=None, bootscript=None):
        args = [
            "-b %s" % build_dir,
            "-s %s" % source_dir
        ]

        if compile_:
            args.append("--compile")
            # args.append("--bin %s" % self.get_lib_path(self.BUILD_DEVELOP))

        if continue_:
            args.append("--continue")

        if wait:
            args.append("-wait")

        if bootscript:
            args.append("--bootscript %s" % bootscript)

        if daemon:
            args.append("--daemon")

        if port:
            args.append("--port %s" % port)

        # TODO bug #114 workaround. Disable create sub engine...
        if wid and platform.system().lower() != 'darwin':
            args.append("--wid %s" % int(wid))

        if core_dir:
            args.append("--core %s" % core_dir)
        else:
            args.append("--core ../core")  # TODO ?

        self._run_cetech(self.BUILD_DEVELOP, args)

    def _run_cetech(self, build_type, args):
        if platform.system().lower() != 'windows':
            cmd = "mono %s %s" % (self.get_executable_path(build_type), ' '.join(args))
        else:
            cmd = "%s %s" % (self.get_executable_path(build_type), ' '.join(args))

        process = QProcess()

        if platform.system().lower() != 'windows':
            process_env = QProcessEnvironment.systemEnvironment()
            process_env.insert("LD_LIBRARY_PATH", self.get_lib_path(build_type))
            process.setProcessEnvironment(process_env)

        process.start(cmd)
        process.waitForStarted()

        self.process = process

    def kill(self, dump=False):
        if dump:
            self._dump()

        self.process.terminate()

    def _dump(self):
        p = self.process
        out, err = bytearray(p.readAllStandardOutput()).decode(), bytearray(p.readAllStandardError()).decode()

        print("=== Process: %s ===" % self.name)
        print("STDOUT:")
        print(out)
        print("STDERR:")
        print(err)
        print("===================%s\n" % ('=' * len(self.name)))


class Project(object):
    def __init__(self):
        self.name = None
        self.project_dir = None
        self.spawned_process = []
        self.instances = {}

    def open_project(self, name, project_dir):
        if project_dir == '':
            return False

        if name == '':
            return False

        if self.is_project_opened():
            self.close_project()

        if not validate_project(project_dir):
            return False

        self.project_dir = project_dir
        self.name = name

        return True

    @property
    def source_dir(self):
        return os.path.join(self.project_dir, "src")

    @property
    def build_dir(self):
        return os.path.join(self.project_dir, "build")

    def close_project(self):
        self.project_dir = None

    def is_project_opened(self):
        return self.project_dir is not None

    def open_project_dialog(self, parent):
        project_dir = QFileDialog.getExistingDirectory(parent, "Select project directory ...")
        return self.open_project(project_dir)

    def killall(self, dump=False):
        for instance in self.instances.values():
            instance.kill(dump=dump)

    def run_cetech_release(self, name):
        instance = EngineInstance(name)
        self.instances[name] = instance

        instance.run_cetech_release(self.build_dir)

        return instance

    def run_cetech_develop(self, name, compile_=False, continue_=False, wait=False, daemon=False, wid=None,
                           core_dir=None, port=None, bootscript=None):

        instance = EngineInstance(name)
        self.instances[name] = instance

        instance.run_cetech_develop(self.build_dir, self.source_dir, compile_=compile_, continue_=continue_, wait=wait,
                                    daemon=daemon, wid=wid,
                                    core_dir=core_dir, port=port, bootscript=bootscript)

        return instance
