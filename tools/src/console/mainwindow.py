from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QMainWindow, QTreeWidgetItem

from cetech.api import ConsoleAPI
from console.ui.mainwindow import Ui_MainWindow


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)

        self.centralwidget.hide()

        self.api = ConsoleAPI('localhost', 2222)
        self.api.register_on_log_handler(self.add_log)

        self.console_api_timer = QTimer()
        self.console_api_timer.timeout.connect(self.api.tick)
        self.console_api_timer.start(10)

    def closeEvent(self, evnt):
        self.api.disconnect()

        self.statusbar.showMessage("Disconnecting ...")

        while(self.api.connect):
            self.api.tick()

        self.statusbar.showMessage("Disconnected")

        super(MainWindow, self).closeEvent(evnt)


    LOG_COLOR = {
        'I': QColor("blue"),
        'W': QColor("yellow"),
        'D': QColor("green"),
        'E': QColor("red"),
    }

    LOG_COLOR_TAG = {
        'I': "%s",
        'W': "<span style='color: yellow;'> %s</span>",
        'D': "%s",
        'E': "<span style='color: red;'> %s</span>",
    }

    def add_log(self, level, where, message):
        item = QTreeWidgetItem([level, where, message])
        self.LogWidget.addTopLevelItem(item)

        if where == 'lua':
            self.command_log_textedit.append(self.LOG_COLOR_TAG[level] % message)

    def send_command(self):
        text = self.command_lineedit.text()

        if len(text) > 0:
            self.api.lua_execute(text)
            self.command_log_textedit.append("> %s" % text)

        self.command_lineedit.setText("")
