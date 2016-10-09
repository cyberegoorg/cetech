from PyQt5.QtCore import Qt, QFileSystemWatcher, QDirIterator
from PyQt5.QtWidgets import QMainWindow, QTabWidget

from playground.core.modules import ModuleManager
from playground.core.projectmanager import ProjectManager
from playground.modules.assetbrowser import AssetBrowser
from playground.modules.assetviewwidget import AssetViewWidget
from playground.modules.levelwidget import LevelWidget
from playground.modules.logwidget import LogWidget
from playground.modules.profilerwidget import ProfilerWidget
from playground.modules.scripteditor import ScriptEditorWidget, ScriptEditorManager
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
            LevelWidget,
            AssetViewWidget,
            AssetBrowser,
            LogWidget,
            ProfilerWidget,
            ScriptEditorManager
        ])

        self.modules_manager['asset_browser'].widget.open_asset.connect(self.open_asset)
        self.modules_manager['asset_browser'].widget.show_asset.connect(self.show_asset)

        self.file_watch = QFileSystemWatcher(self)
        self.file_watch.fileChanged.connect(self.file_changed)
        self.file_watch.directoryChanged.connect(self.dir_changed)

        self.build_file_watch = QFileSystemWatcher(self)
        self.build_file_watch.fileChanged.connect(self.build_file_changed)
        self.build_file_watch.directoryChanged.connect(self.build_dir_changed)

    def open_asset(self, path, ext):
        """
        :type path: str
        :type path: ext
        """
        # TODO: move fce to asset view

        asset_name = path.replace(self.project.project_dir, '').replace('/src/', '').split('.')[0]

        if ext == 'level':
            if not self.modules_manager['level_editor'].widget.instance.ready:
                return

            self.modules_manager['level_editor'].widget.open_level(asset_name)
            return

        if ScriptEditorWidget.support_ext(ext):
            self.modules_manager['script_editor'].widget.open(self, path)

    def show_asset(self, path, ext):
        asset_name = path.replace(self.project.project_dir, '').replace('/src/', '').split('.')[0]
        self.modules_manager['asset_view'].widget.open_asset(asset_name, ext)

    def open_project(self, name, dir):
        self.project.open_project(name, dir)
        self.modules_manager.open_project(self.project)
        self.watch_project_dir()

    def reload_all(self):
        # TODO: implement reload all
        order = ["shader", "texture", "material", "lua"]
        # self.api.reload_all(order)
        # self.assetview_widget.api.reload_all(order)
        pass

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
        # TODO: self.api.compile_all()
        pass

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
        self.modules_manager['level_editor'].widget.close_project()
        self.modules_manager['asset_view'].widget.close_project()

        self.project.killall()
        evnt.accept()
