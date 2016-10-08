from collections import OrderedDict

from PyQt5.QtCore import Qt, QFileSystemWatcher, QDirIterator
from PyQt5.QtWidgets import QMainWindow, QDockWidget, QTabWidget

from playground.core.projectmanager import ProjectManager
from playground.modules.assetbrowser import AssetBrowser
from playground.modules.assetviewwidget import AssetViewWidget
from playground.modules.levelwidget import LevelWidget
from playground.modules.logwidget import LogWidget
from playground.modules.profilerwidget import ProfilerWidget
from playground.modules.scripteditor import ScriptEditorWidget, ScriptEditorManager
from playground.ui.mainwindow import Ui_MainWindow


class ModuleInstance(object):
    def __init__(self, widget, dock):
        self.widget = widget
        self.dock = dock


class ModuleManager(object):
    def __init__(self, main_window):
        self.modules_instance = OrderedDict()
        self.main_window = main_window

    def new_docked(self, module_instance, name, title, dock_area):
        dock = QDockWidget()
        dock.setWindowTitle(title)
        dock.hide()
        dock.setWidget(module_instance)
        dock.show()
        self.main_window.addDockWidget(dock_area, dock)

        self.modules_instance[name] = ModuleInstance(module_instance, dock)

    def __getitem__(self, item):
        return self.modules_instance[item]

    def open_project(self, project):
        for v in self.modules_instance.values():
            v.widget.open_project(project)


MODULES = [
    LevelWidget,
    AssetViewWidget,
    AssetBrowser,
    LogWidget
]


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)

        self.modules_manager = ModuleManager(self)

        self.centralwidget.hide()

        self.project = ProjectManager()

        self.setTabPosition(Qt.AllDockWidgetAreas, QTabWidget.North)

        for module in MODULES:
            module(module_manager=self.modules_manager)

        self.modules_manager['asset_browser'].widget.open_asset.connect(self.open_asset)
        self.modules_manager['asset_browser'].widget.show_asset.connect(self.show_asset)

        self.editors_manager = ScriptEditorManager(project_manager=self.project, module_manager=self.modules_manager)

        self.profiler_widget = ProfilerWidget()
        self.profiler_doc_widget = QDockWidget(self)
        self.profiler_doc_widget.setWindowTitle("Profiler")
        self.profiler_doc_widget.hide()
        self.profiler_doc_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.profiler_doc_widget.setWidget(self.profiler_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.profiler_doc_widget)

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
            self.editors_manager.open(self, path)

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
        self.profiler_doc_widget.show()

    def run_standalone(self):
        self.project.run_release("Standalone")

    def run_level(self):
        self.project.run_develop("Level", compile_=True, continue_=True, port=5566)

    def closeEvent(self, evnt):
        self.modules_manager['level_editor'].widget.close_project()
        self.modules_manager['asset_view'].widget.close_project()

        self.project.killall()
        evnt.accept()
