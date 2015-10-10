import os
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
    BUILD_DEBUG = 'cetech1_debug'
    BUILD_DEVELOP = 'cetech1_develop'
    BUILD_RELEASE = 'cetech1'

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
        for p in self.spawned_process:
            p.terminate()

    def exec_cmd(self, build_type, *args):
        return '../../engine/.build/linux64/bin/%s %s' % (build_type, ' '.join(args))

    def run_cetech(self, build_type, compile=False, continu=False, wait=True, daemon=False, port=None):
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

        cmd = self.exec_cmd(build_type, *args)

        process = QProcess()
        process.start(cmd)
        process.waitForStarted()

        self.spawned_process.append(process)

        return process
