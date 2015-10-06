from PyQt5.QtCore import QDir, QProcess
from PyQt5.QtWidgets import QFileDialog, QMessageBox


def validate_project(project_dir):
    selected_dir = QDir(project_dir)

    if not selected_dir.exists('src/config.json'):
        QMessageBox.critical(None, 'Project validation error', 'Project dir does not contain src/config.json',
                             QMessageBox.Yes, QMessageBox.Yes)
        return False

    return True


class ProjectManager(object):
    def __init__(self):
        self.project_dir = None
        self.spawned_process = []

    def open_project(self, project_dir):
        if project_dir == '':
            return False

        if self.is_project_opened():
            self.close_project()

        if not validate_project(project_dir):
            return False

        self.project_dir = project_dir

        return True

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

    def run_cetech(self, compile=False, wait=True, daemon=False, port=None):
        args = [
            "-s %s/src/" % self.project_dir,
            "-b %s/build/" % self.project_dir,
        ]

        if compile:
            args.append("-c")

        if wait:
            args.append("-w")

        if port:
            args.append("-p %d" % port)

        if daemon:
            args.append("--daemon")

        cmd = '../../engine/.build/linux64/bin/cetech1_debug %s' % ' '.join(args)

        process = QProcess()
        process.start(cmd)
        process.waitForStarted()

        self.spawned_process.append(process)

        return process
