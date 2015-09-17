import re
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QFrame, QStyle, QTreeWidgetItem

from cetech.proxy import ConsoleProxy
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

    def __init__(self, api: ConsoleProxy, ignore_where=None):
        super(LogWidget, self).__init__()
        self.setupUi(self)

        self.ignore_where = None
        if ignore_where is None:
            ignore_where = ''

        self.set_ignore_where(ignore_where)

        self.api = api
        self.api.register_handler('log', self.add_log)

    def set_ignore_where(self, pattern):
        self.ignore_where = re.compile(pattern)

    def _is_ignored(self, name):
        m = self.ignore_where.match(name)
        return m is not None

    def add_log(self, time, level, where, msg):
        if self._is_ignored(where):
            return

        item = QTreeWidgetItem(['', where, msg])

        item.setIcon(0, self.style().standardIcon(self.LOG_ICON[level]))

        self.log_tree_widget.addTopLevelItem(item)

        sb = self.log_tree_widget.verticalScrollBar()
        sb.setValue(sb.maximum())