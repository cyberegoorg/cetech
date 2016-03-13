import argparse
import platform

import time
from PyQt5.QtCore import QThread, Qt, QFileSystemWatcher, QDirIterator
from PyQt5.QtWidgets import QMainWindow, QDockWidget, QTabWidget

from playground.assetbrowser import AssetBrowser
from playground.engine.cetechproject import CetechProject
from playground.engine.cetechwidget import CetechWidget
from playground.engine.consoleapi import ConsoleAPI
from playground.logwidget import LogWidget, LogSub
from playground.profilerwidget import ProfilerWidget
from playground.scripteditor import ScriptEditorWidget, ScriptEditorManager
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

        self.project = CetechProject()

        self.api = ConsoleAPI(b"tcp://localhost:5557")

        self.setTabPosition(Qt.AllDockWidgetAreas, QTabWidget.North)

        self.editors_manager = ScriptEditorManager(project_manager=self.project, api=self.api)

        # TODO bug #114 workaround. Disable create sub engine...
        if platform.system().lower() != 'darwin':
            self.ogl_widget = CetechWidget(self, self.api, self.project)
            self.ogl_dock = QDockWidget(self)
            self.ogl_dock.setWindowTitle("Engine View")
            self.ogl_dock.hide()
            self.ogl_dock.setWidget(self.ogl_widget)
            self.addDockWidget(Qt.TopDockWidgetArea, self.ogl_dock)

        self.assetb_widget = AssetBrowser()
        self.assetb_dock_widget = QDockWidget(self)
        self.assetb_dock_widget.hide()
        self.assetb_dock_widget.setWindowTitle("Asset browser")
        self.assetb_dock_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.assetb_dock_widget.setWidget(self.assetb_widget)
        self.addDockWidget(Qt.BottomDockWidgetArea, self.assetb_dock_widget)

        self.profiler_widget = ProfilerWidget(api=self.api)
        self.profiler_doc_widget = QDockWidget(self)
        self.profiler_doc_widget.setWindowTitle("Profiler")
        self.profiler_doc_widget.hide()
        self.profiler_doc_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.profiler_doc_widget.setWidget(self.profiler_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.profiler_doc_widget)

        self.logsub = LogSub(b"ws://localhost:5556")
        self.log_widget = LogWidget(None, self.logsub)  # TODO: open file
        self.log_dock_widget = QDockWidget(self)
        self.log_dock_widget.hide()
        self.log_dock_widget.setWindowTitle("Log")
        self.log_dock_widget.setWidget(self.log_widget)

        self.addDockWidget(Qt.BottomDockWidgetArea, self.log_dock_widget)
        self.tabifyDockWidget(self.assetb_dock_widget, self.log_dock_widget)

        self.assetb_widget.asset_clicked.connect(self.open_asset)

        self.file_watch = QFileSystemWatcher(self)
        self.file_watch.fileChanged.connect(self.file_changed)
        self.file_watch.directoryChanged.connect(self.dir_changed)

        self.build_file_watch = QFileSystemWatcher(self)
        self.build_file_watch.fileChanged.connect(self.build_file_changed)
        self.build_file_watch.directoryChanged.connect(self.build_dir_changed)

    def open_asset(self, path, ext):
        if ScriptEditorWidget.support_ext(ext):
            self.editors_manager.open(self, path)

    def open_project(self, name, dir):
        self.project.open_project(name, dir)

        if platform.system().lower() == 'darwin':
            wid = None
        else:
            wid = self.ogl_widget.winId()

        self.project.run_cetech(build_type=CetechProject.BUILD_DEBUG, compile_=True, continue_=True,
                                wid=wid)

        time.sleep(5)

        self.api.connect()
        self.api.wait()

        #self.api.start(QThread.LowPriority)
        self.logsub.start(QThread.LowPriority)

        self.assetb_widget.open_project(self.project.project_dir)

        self.log_dock_widget.show()

        # TODO bug #114 workaround. Disable create sub engine...
        if platform.system().lower() != 'darwin':
            self.ogl_dock.show()

        self.watch_project_dir()

        self.ogl_widget.resize_event_enable = True
        self.ogl_widget.resize(self.ogl_widget.size())

        self.assetb_dock_widget.show()

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
        self.api.compile_all()

    def dir_changed(self, path):
        self.watch_project_dir()

    def build_file_changed(self, path):
        self.api.autocomplete_list()

    def build_dir_changed(self, path):
        pass

    def open_script_editor(self):
        self.script_editor_dock_widget.show()

    def open_recorded_events(self):
        self.profiler_doc_widget.show()

    def closeEvent(self, evnt):
        self.api.disconnect()
        self.project.killall_process()
        evnt.accept()
