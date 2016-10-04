import argparse

from PyQt5.QtCore import QThread, Qt, QFileSystemWatcher, QDirIterator
from PyQt5.QtWidgets import QMainWindow, QDockWidget, QTabWidget

from playground.core.projectmanager import ProjectManager
from playground.modules.assetbrowser import AssetBrowser
from playground.modules.assetviewwidget import AssetViewWidget
from playground.modules.levelwidget import LevelWidget
from playground.modules.logwidget import LogWidget, LogSub
from playground.modules.profilerwidget import ProfilerWidget
from playground.modules.scripteditor import ScriptEditorWidget, ScriptEditorManager
from playground.ui.mainwindow import Ui_MainWindow


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)

        self.centralwidget.hide()

        self.parser = argparse.ArgumentParser("playground")
        self.parser.add_argument("-d", "--data-dir", type=str, help="data dir")
        self.parser.add_argument("-a", "--console-address", type=str, help="console address", default='localhost')
        self.parser.add_argument("-p", "--console-port", type=int, help="console port", default=2222)
        self.args = self.parser.parse_args()

        self.data_dir = self.args.data_dir
        self.console_port = self.args.console_port
        self.console_address = self.args.console_address

        self.project = ProjectManager()

        self.setTabPosition(Qt.AllDockWidgetAreas, QTabWidget.North)

        self.editors_manager = ScriptEditorManager(project_manager=self.project)

        self.level_widget = LevelWidget(self)
        self.level_dock = QDockWidget(self)
        self.level_dock.setWindowTitle("Level view")
        self.level_dock.hide()
        self.level_dock.setWidget(self.level_widget)
        self.addDockWidget(Qt.TopDockWidgetArea, self.level_dock)

        self.assetview_widget = AssetViewWidget(self)
        self.assetview_dock = QDockWidget(self)
        self.assetview_dock.setWindowTitle("Asset view")
        self.assetview_dock.setWidget(self.assetview_widget)
        self.addDockWidget(Qt.BottomDockWidgetArea, self.assetview_dock)

        self.assetb_widget = AssetBrowser()
        self.assetb_dock_widget = QDockWidget(self)
        self.assetb_dock_widget.hide()
        self.assetb_dock_widget.setWindowTitle("Asset browser")
        self.assetb_dock_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.assetb_dock_widget.setWidget(self.assetb_widget)
        self.addDockWidget(Qt.BottomDockWidgetArea, self.assetb_dock_widget)

        self.splitDockWidget(self.assetb_dock_widget, self.assetview_dock, Qt.Horizontal)

        self.profiler_widget = ProfilerWidget()
        self.profiler_doc_widget = QDockWidget(self)
        self.profiler_doc_widget.setWindowTitle("Profiler")
        self.profiler_doc_widget.hide()
        self.profiler_doc_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.profiler_doc_widget.setWidget(self.profiler_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.profiler_doc_widget)

        self.log_widget = LogWidget()
        self.log_dock_widget = QDockWidget(self)
        self.log_dock_widget.hide()
        self.log_dock_widget.setWindowTitle("Log")
        self.log_dock_widget.setWidget(self.log_widget)

        self.addDockWidget(Qt.BottomDockWidgetArea, self.log_dock_widget)
        self.tabifyDockWidget(self.assetb_dock_widget, self.log_dock_widget)

        self.assetb_widget.asset_doubleclicked.connect(self.open_asset)
        self.assetb_widget.asset_clicked.connect(self.show_asset)

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

        if not self.level_widget.instance.ready:
            return

        asset_name = path.replace(self.project.project_dir, '').replace('/src/', '').split('.')[0]

        if ext == 'level':
            self.level_widget.open_level(asset_name)
            return

        if ScriptEditorWidget.support_ext(ext):
            self.editors_manager.open(self, path)

    def show_asset(self, path, ext):
        asset_name = path.replace(self.project.project_dir, '').replace('/src/', '').split('.')[0]
        self.assetview_widget.open_asset(asset_name, ext)

    def open_project(self, name, dir):
        self.project.open_project(name, dir)

        self.assetb_widget.open_project(self.project.project_dir)
        self.log_dock_widget.show()
        self.assetb_dock_widget.show()
        self.level_dock.show()

        self.level_widget.open_project(self.project)
        self.log_widget.set_instance(self.level_widget.instance)
        self.assetview_widget.open_project(self.project)

        self.watch_project_dir()

    def reload_all(self):
        # TODO: implement reload all
        order = ["shader", "texture", "material", "lua"]
        # self.api.reload_all(order)
        self.assetview_widget.api.reload_all(order)

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
        self.level_widget.close_project()
        self.assetview_widget.close_project()

        self.project.killall()
        evnt.accept()
