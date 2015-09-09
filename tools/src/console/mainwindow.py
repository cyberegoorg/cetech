import argparse
from PyQt5.QtCore import QTimer
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QMainWindow, QTreeWidgetItem, QStyle

from cetech.api import ConsoleAPI
from console.consolewidget import ConsoleWidget
from console.ui.mainwindow import Ui_MainWindow


class MainWindow(QMainWindow, Ui_MainWindow):
    LOG_COLOR = {
        'I': QColor("blue"),
        'W': QColor("yellow"),
        'D': QColor("green"),
        'E': QColor("red"),
    }

    LOG_ICON = {
        'I': QStyle.SP_MessageBoxInformation,
        'W': QStyle.SP_MessageBoxWarning,
        'D': QStyle.SP_MessageBoxQuestion,
        'E': QStyle.SP_MessageBoxCritical,
    }

    LOG_COLOR_TAG = {
        'I': "%s",
        'W': "<span style='color: yellow;'> %s</span>",
        'D': "%s",
        'E': "<span style='color: red;'> %s</span>",
    }

    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)

        self.centralwidget.hide()

        self.parser = argparse.ArgumentParser("cetech_console")
        self.parser.add_argument("-d", "--data-dir", type=str, help="data dir")
        self.parser.add_argument("-a", "--console-address", type=str, help="console address", default='localhost')
        self.parser.add_argument("-p", "--console-port", type=int, help="console port", default=2222)
        self.args = self.parser.parse_args()

        self.data_dir = self.args.data_dir
        self.console_port = self.args.console_port
        self.console_address = self.args.console_address

        self.api = ConsoleAPI(self.console_address, self.console_port)
        self.api.register_on_log_handler(self.add_log)

        self.console_widget = ConsoleWidget(self.api)
        self.console_dock_widget.setWidget(self.console_widget)

        self.console_api_timer = QTimer()
        self.console_api_timer.timeout.connect(self.api.tick)
        self.console_api_timer.start(10)

    def closeEvent(self, evnt):
        self.api.disconnect()

        self.statusbar.showMessage("Disconnecting ...")

        while (self.api.connect):
            self.api.tick()

        self.statusbar.showMessage("Disconnected")

        super(MainWindow, self).closeEvent(evnt)

    def add_log(self, level, where, message):
        item = QTreeWidgetItem([level, where, message])

        item.setIcon(0, self.style().standardIcon(self.LOG_ICON[level]))

        self.LogWidget.addTopLevelItem(item)
