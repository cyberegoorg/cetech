import os
import platform

from PyQt5.QtCore import QDir
from PyQt5.QtWidgets import QFileDialog, QMessageBox

from cetech.engine import EngineInstance


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


ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir, os.pardir)


class ProjectManager(object):
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

    def run_release(self, name):
        instance = EngineInstance(name, "../bin", os.path.join(ROOT_DIR, 'externals', 'build'))
        self.instances[name] = instance

        instance.run_release(self.build_dir)

        return instance

    def run_develop(self, name, compile_=False, continue_=False, wait=False, daemon=False, wid=None,
                    core_dir=None, port=None, bootscript=None):

        instance = EngineInstance(name, "../bin", os.path.join(ROOT_DIR, 'externals', 'build'))
        self.instances[name] = instance

        instance.run_develop(self.build_dir, self.source_dir, compile_=compile_, continue_=continue_, wait=wait,
                             daemon=daemon, wid=wid,
                             core_dir=core_dir, port=port, bootscript=bootscript)

        return instance
