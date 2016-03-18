import os
import platform

import time
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


class CetechProject(object):
    BUILD_DEBUG = 'Debug'
    BUILD_DEVELOP = 'Develop'
    BUILD_RELEASE = 'Release'

    _BIN_PLATFORM = {
        'windows': 'Windows',
        'linux': 'Linux'
    }

    _BUILD_DIR = {
        BUILD_DEBUG: 'Debug',
        BUILD_DEVELOP: 'Debug',
        BUILD_RELEASE: 'Release'
    }

    def __init__(self):
        self.project_dir = None
        self.spawned_process = []

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

    def killall_process(self):
        self.dump()
        for p in self.spawned_process:
            p.terminate()

    def dump(self):
        for p in self.spawned_process:
            out, err = bytearray(p.readAllStandardOutput()).decode(), bytearray(p.readAllStandardError()).decode()
            print("out:\n%s\n err:\n%s\n" % (out, err))

    def get_lib_path(self, build_type):
        _platform = platform.system().lower()

        engine_bin_path = os.path.join('..', 'sources', 'CETech', 'bin', self._BIN_PLATFORM[_platform], 'AnyCPU',
                                       self._BUILD_DIR[build_type])
        return engine_bin_path

    def get_executable_path(self, build_type):
        _platform = platform.system().lower()

        engine_bin_path = os.path.join('..', 'sources', 'CETech', 'bin', self._BIN_PLATFORM[_platform], 'AnyCPU',
                                       self._BUILD_DIR[build_type], 'CETech.exe')
        return engine_bin_path

    def run_cetech(self, build_type, compile_=False, continue_=False, wait=False, daemon=False, wid=None,
                   core_dir=None):
        args = [
            "-s %s" % self.source_dir,
            "-b %s" % self.build_dir,
        ]

        if compile_:
            args.append("--compile")

        if continue_:
            args.append("--continue")

        if wait:
            args.append("-wait")

        if daemon:
            args.append("--daemon")

        # TODO bug #114 workaround. Disable create sub engine...
        if wid and platform.system().lower() != 'darwin':
            args.append("--wid %s" % int(wid))

        if core_dir:
            args.append("--core %s" % core_dir)
        else:
            args.append("--core ../core")

        cmd = "%s %s" % (self.get_executable_path(build_type), ' '.join(args))

        process = QProcess()

        if platform.system().lower() != 'windows':
            process_env = QProcessEnvironment.systemEnvironment()
            process_env.insert("LD_LIBRARY_PATH", self.get_lib_path(build_type))
            process.setProcessEnvironment(process_env)

        process.start(cmd)
        process.waitForStarted()

        self.spawned_process.append(process)

        return process
