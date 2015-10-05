import argparse

from PyQt5.QtCore import QThread, Qt
from PyQt5.QtWidgets import QMainWindow, QDockWidget
from cetech.qtapi import QtConsoleAPI
from playground.projectmanager import ProjectManager
from playground.ui.mainwindow import Ui_MainWindow
from shared.logwidget import LogWidget
from shared.luaeditor import LuaEditor
from shared.replwidget import REPLWidget


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

        self.api = QtConsoleAPI(self.console_address, self.console_port)

        self.log_widget = LogWidget(self.api, ignore_where='lua\.*.*')
        self.log_dock_widget.setWidget(self.log_widget)

        self.repl_widget = REPLWidget(self.api)
        self.repl_dock_widget = QDockWidget(self)
        self.repl_dock_widget.hide()
        self.repl_dock_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.repl_dock_widget.setWidget(self.repl_widget)
        self.addDockWidget(Qt.TopDockWidgetArea, self.repl_dock_widget)

        self.lua_editor_widget = LuaEditor(project_manager=self.project)
        self.lua_editor_dock_widget = QDockWidget(self)
        self.lua_editor_dock_widget.setWindowTitle("Lua editor")
        self.lua_editor_dock_widget.hide()
        self.lua_editor_dock_widget.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.lua_editor_dock_widget.setWidget(self.lua_editor_widget)
        self.addDockWidget(Qt.TopDockWidgetArea, self.lua_editor_dock_widget)


    def open_project(self):
        self.project.open_project_dialog(self)

        self.project.run_cetech(compile=True, daemon=True)
        self.api.start(QThread.LowPriority)

    def open_repl(self):
        self.repl_dock_widget.show()

    def open_lua_editor(self):
        self.lua_editor_dock_widget.show()

    def closeEvent(self, evnt):
        self.api.disconnect()

        self.project.killall_process()

        self.statusbar.showMessage("Disconnecting ...")

        while self.api.connect:
            self.api.tick()

        self.statusbar.showMessage("Disconnected")

        super(MainWindow, self).closeEvent(evnt)
