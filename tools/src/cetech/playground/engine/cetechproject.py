import os
import platform
from PyQt5.QtCore import QDir, QProcess
from PyQt5.QtWidgets import QFileDialog, QMessageBox


def validate_project(project_dir):
    selected_dir = QDir(project_dir)

    if not selected_dir.exists():
        QMessageBox.critical(None, 'Dir does not  exist', 'Dir does not  exist',
                             QMessageBox.Yes, QMessageBox.Yes)
        return False

    if not selected_dir.exists('src/config.json'):
        QMessageBox.critical(None, 'Project validation error', 'Project dir does not contain src/config.json',
                             QMessageBox.Yes, QMessageBox.Yes)
        return False

    return True


class CetechProject(object):
    BUILD_DEBUG = 'debug'
    BUILD_DEVELOP = 'develop'
    BUILD_RELEASE = 'release'

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

    def get_executable_path(self, build_type):
        engine_bin_path = '../../engine/.build/'
        platform_dir = "%s%s" % (platform.system().lower(), platform.architecture()[0][0:2])

        exec_name = "cetech1_%s" % build_type

        return os.path.join(engine_bin_path, platform_dir, 'bin', exec_name)

    def run_cetech(self, build_type, compile=False, continu=False, wait=True, daemon=False, port=None, wid=None,
                   core_dir=None):
        args = [
            "-s %s" % self.source_dir,
            "-b %s" % self.build_dir,
        ]

        if compile:
            args.append("-c")

        if continu:
            args.append("--continue")

        if wait:
            args.append("-w")

        if port:
            args.append("-p %d" % port)

        if daemon:
            args.append("--daemon")

        if wid:
            args.append("--wid %s" % int(wid))

        if core_dir:
            args.append("--core-dir %s" % core_dir)
        else:
            args.append("--core-dir ../../engine/core")

        print(args)
        cmd = "%s %s" % (self.get_executable_path(build_type), ' '.join(args))

        process = QProcess()
        process.start(cmd)
        process.waitForStarted()

        self.spawned_process.append(process)

        return process
