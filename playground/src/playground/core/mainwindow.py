from PyQt5.QtCore import Qt, QFileSystemWatcher, QDirIterator
from PyQt5.QtWidgets import QMainWindow, QTabWidget

from playground.core.modules import ModuleManager
from playground.core.projectmanager import ProjectManager
from playground.modules.assetbrowser import AssetBrowser
from playground.modules.assetviewwidget import AssetViewWidget
from playground.modules.compiler import CompilerModule
from playground.modules.levelwidget import LevelWidget
from playground.modules.logwidget import LogWidget
from playground.modules.profilerwidget import ProfilerWidget
from playground.modules.scripteditor import ScriptEditorManager
from playground.ui.mainwindow import Ui_MainWindow


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)

        self.centralwidget.hide()

        self.project = ProjectManager()
        self.modules_manager = ModuleManager(self)

        self.setTabPosition(Qt.AllDockWidgetAreas, QTabWidget.North)

        self.modules_manager.init_modules([
            CompilerModule,
            LevelWidget,
            AssetViewWidget,
            AssetBrowser,
            LogWidget,
            ProfilerWidget,
            ScriptEditorManager
        ])

        self.file_watch = QFileSystemWatcher(self)
        self.file_watch.fileChanged.connect(self.file_changed)
        self.file_watch.directoryChanged.connect(self.dir_changed)

        self.build_file_watch = QFileSystemWatcher(self)
        self.build_file_watch.fileChanged.connect(self.build_file_changed)
        self.build_file_watch.directoryChanged.connect(self.build_dir_changed)

    def open_project(self, name, dir):
        self.project.open_project(name, dir)
        self.modules_manager.open_project(self.project)
        self.watch_project_dir()

    def reload_all(self):
        self.modules_manager['compiler'].widget.compile_all()

        for k, v in self.project.instances.items():
            v.console_api.reload_all()

    def watch_project_dir(self):
        files = self.file_watch.files()
        directories = self.file_watch.directories()

        if len(files):
            self.file_watch.removePaths(files)

        if len(directories):
            self.file_watch.removePaths(directories)

        files = self.build_file_watch.files()
        directories = self.build_file_watch.directories()

        if len(files):
            self.build_file_watch.removePaths(files)

        if len(directories):
            self.build_file_watch.removePaths(directories)

        files = []
        it = QDirIterator(self.project.source_dir, QDirIterator.Subdirectories)
        while it.hasNext():
            files.append(it.next())

        self.file_watch.addPaths(files)

        files = []
        it = QDirIterator(self.project.build_dir, QDirIterator.Subdirectories)
        while it.hasNext():
            files.append(it.next())

        self.build_file_watch.addPaths(files)

    def file_changed(self, path):
        self.modules_manager['compiler'].widget.compile_all()

    def dir_changed(self, path):
        self.watch_project_dir()

    def build_file_changed(self, path):
        pass

    def build_dir_changed(self, path):
        pass

    def open_script_editor(self):
        self.script_editor_dock_widget.show()

    def open_recorded_events(self):
        self.modules_manager['profiler'].dock.show()

    def run_standalone(self):
        self.project.run_release("Standalone")

    def run_level(self):
        self.project.run_develop("Level", compile_=True, continue_=True, port=5566)

    def closeEvent(self, evnt):
        self.modules_manager.close_project()
        self.project.killall()
        evnt.accept()
