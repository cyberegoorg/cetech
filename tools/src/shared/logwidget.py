from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QFrame, QStyle, QTreeWidgetItem
import re

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

    def __init__(self, api, ingore_where=None):
        super(LogWidget, self).__init__()
        self.setupUi(self)

        if ingore_where is None:
            ingore_where = ''

        self.set_ingore_where(ingore_where)

        self.api = api
        self.api.register_on_log_handler(self.add_log)

    def set_ingore_where(self, pattern):
        self.ingore_where = re.compile(pattern)

    def _disabled(self, name):
        m = self.ingore_where.match(name)
        return m is not None

    def add_log(self, level, where, message):
        if self._disabled(where):
            return

        item = QTreeWidgetItem([level, where, message])

        item.setIcon(0, self.style().standardIcon(self.LOG_ICON[level]))

        self.log_tree_widget.addTopLevelItem(item)
