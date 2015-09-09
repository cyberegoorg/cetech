from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QFrame, QStyle, QTreeWidgetItem

from shared.ui.logwidget import Ui_LogWidget


class LogWidget(QFrame, Ui_LogWidget):
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

    def __init__(self, api):
        super(LogWidget, self).__init__()
        self.setupUi(self)

        self.api = api
        self.api.register_on_log_handler(self.add_log)

    def add_log(self, level, where, message):
        item = QTreeWidgetItem([level, where, message])

        item.setIcon(0, self.style().standardIcon(self.LOG_ICON[level]))

        self.log_tree_widget.addTopLevelItem(item)
