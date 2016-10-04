import argparse
import platform

import msgpack
import nanomsg
from PyQt5.QtCore import QThread, Qt, QFileSystemWatcher, QDirIterator
from PyQt5.QtWidgets import QMainWindow, QDockWidget, QTabWidget
from nanomsg import Socket, SUB, SUB_SUBSCRIBE

from cetech.consoleapi import ConsoleAPI
from playground.assetbrowser import AssetBrowser
from playground.assetview import AssetView
from playground.logwidget import LogWidget, LogSub
from playground.profilerwidget import ProfilerWidget
from playground.project import Project
from playground.scripteditor import ScriptEditorWidget, ScriptEditorManager
from playground.ui.mainwindow import Ui_MainWindow
from playground.widget import CETechWiget


class DevelopSub(QThread):
    def __init__(self, url):
        self.socket = Socket(SUB)
        self.url = url
        self.handlers = []

        super().__init__()

    def register_handler(self, handler):
        self.handlers.append(handler)

    def run(self):
        self.socket.set_string_option(SUB, SUB_SUBSCRIBE, b'')
        self.socket.connect(self.url)
        while True:
            try:
                msg = self.socket.recv()
                unpack_msg = msgpack.unpackb(msg, encoding='utf-8')
                print(unpack_msg)

            except nanomsg.NanoMsgAPIError as e:
                raise


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

        self.project = Project()

        self.api = ConsoleAPI("ws://localhost:4444")

        # self.develop_sub = DevelopSub(b"ws://localhost:5558")
        # self.develop_sub.run()

        self.setTabPosition(Qt.AllDockWidgetAreas, QTabWidget.North)

        self.editors_manager = ScriptEditorManager(project_manager=self.project, api=self.api)

        # TODO bug #114 workaround. Disable create sub engine...
        if platform.system().lower() != 'darwin':
            self.ogl_widget = CETechWiget(self, self.api, log_url=b"ws://localhost:5556")
            self.ogl_dock = QDockWidget(self)
            self.ogl_dock.setWindowTitle("Engine View")
            self.ogl_dock.hide()
            self.ogl_dock.setWidget(self.ogl_widget)
            self.addDockWidget(Qt.TopDockWidgetArea, self.ogl_dock)

        self.assetview_widget = AssetView(self)
        self.assetview_dock = QDockWidget(self)
        self.assetview_dock.setWindowTitle("Asset view")
        # self.assetview_dock.hide()
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

        self.profiler_widget = ProfilerWidget(api=self.api)
        self.profiler_doc_widget = QDockWidget(self)
        self.profiler_doc_widget.setWindowTitle("Profiler")
        self.profiler_doc_widget.hide()
        self.profiler_doc_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.profiler_doc_widget.setWidget(self.profiler_widget)
        self.addDockWidget(Qt.RightDockWidgetArea, self.profiler_doc_widget)

        self.logsub = LogSub(b"ws://localhost:4445")
        self.log_widget = LogWidget(None, self.logsub)  # TODO: open file
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

        asset_name = path.replace(self.project.project_dir, '').replace('/src/', '').split('.')[0]

        if ext == 'level':
            level_name = asset_name
            self.api.lua_execute("Editor:load_level(\"%s\")" % level_name)
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

        self.watch_project_dir()

        self.assetb_dock_widget.show()

        if platform.system().lower() == 'darwin':
            wid = None
        else:
            wid = self.ogl_widget.winId()
            # wid = None

        # TODO bug #114 workaround. Disable create sub engine...
        if platform.system().lower() != 'darwin':
            self.ogl_dock.show()

        self.api.connect()
        self.logsub.start(QThread.NormalPriority)

        self.ogl_widget.set_instance(
            self.project.run_develop("LevelView", compile_=True, continue_=True, wid=wid,
                                     bootscript="playground/leveleditor_boot"))

        self.assetview_widget.open_project(self.project)
        # self.api.wait()

    def reload_all(self):
        order = ["shader", "texture", "material", "lua"]
        self.api.reload_all(order)
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
        self.api.compile_all()

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
        self.api.quit()
        self.api.disconnect()
        self.assetview_widget.close_project()
        self.project.killall()
        evnt.accept()
