import re

from PyQt5.QtCore import QDateTime, Qt
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QFrame, QStyle, QTreeWidgetItem
from playground.ui.logwidget import Ui_LogWidget

RESOURCE_NAME_RE = re.compile("^\[([^\]]+)\]")

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


class LogWidget(QFrame, Ui_LogWidget):
    def __init__(self, api, script_editor, ignore_where=None):
        """

        :type api: ConsoleProxy
        """
        super(LogWidget, self).__init__()
        self.setupUi(self)

        self.script_editor = script_editor

        self.ignore_where = None
        if ignore_where is not None:
            self.set_ignore_where(ignore_where)

        self.api = api
        self.api.register_handler('log', self.add_log)

        self.log_tree_widget.header().setStretchLastSection(True)

    def set_ignore_where(self, pattern):
        self.ignore_where = re.compile(pattern)

    def _is_ignored(self, name):
        if not self.ignore_where:
            return False

        m = self.ignore_where.match(name)
        return m is not None

    def add_log(self, time, level, worker_id, where, msg):
        if self._is_ignored(where):
            return

        dt_s = QDateTime.fromTime_t(time).toString("hh:mm:ss.zzz")

        item = QTreeWidgetItem(['', dt_s, str(worker_id), where, msg])

        item.setIcon(0, self.style().standardIcon(LOG_ICON[level]))
        item.setData(0, Qt.UserRole, level)

        self.log_tree_widget.addTopLevelItem(item)

        sb = self.log_tree_widget.verticalScrollBar()
        sb.setValue(sb.maximum())

    def item_double_clicked(self, item, idx):
        """

        :type item: QTreeWidgetItem
        """

        level = item.data(0, Qt.UserRole)
        if level != 'E':
            return

        where = item.text(3)
        if 'resource_compiler' not in where:
            return

        msg = item.text(4)

        resource_name = RESOURCE_NAME_RE.match(msg).group(1)
        self.script_editor.open_resource(resource_name)

