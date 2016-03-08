import re

import yaml
from PyQt5.QtCore import QDateTime, Qt, QThread
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QFrame, QStyle, QTreeWidgetItem

import nanomsg
from nanomsg import Socket, SUB, SUB_SUBSCRIBE, DONTWAIT

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


class LogSub(QThread):
    def __init__(self, url):
        self.socket = Socket(SUB)
        self.url = url
        self.handlers = []

        super().__init__()

    def register_handler(self, handler):
        self.handlers.append(handler)

    def run(self):
        #self.socket.set_string_option(SUB, SUB_SUBSCRIBE, b'')
        self.socket.connect(self.url)
        while True:
            try:
                msg = self.socket.recv()
                print(msg)
                msg_yaml = yaml.load(msg)
                for h in self.handlers:
                    h(**msg_yaml)

            except nanomsg.NanoMsgAPIError as e:
                raise

class LogWidget(QFrame, Ui_LogWidget):
    def __init__(self, script_editor, logsub, ignore_where=None):
        super(LogWidget, self).__init__()
        self.setupUi(self)

        self.script_editor = script_editor

        self.ignore_where = None
        if ignore_where is not None:
            self.set_ignore_where(ignore_where)

        logsub.register_handler(self.add_log)

        self.log_tree_widget.header().setStretchLastSection(True)

    def set_ignore_where(self, pattern):
        self.ignore_where = re.compile(pattern)

    def _is_ignored(self, name):
        if not self.ignore_where:
            return False

        m = self.ignore_where.match(name)
        return m is not None

    def add_log(self, time, level, worker, where, msg):
        #print(time, level, worker, where, msg)
        if self._is_ignored(where):
            return

        dt_s = QDateTime.fromTime_t(time).toString("hh:mm:ss.zzz")

        item = QTreeWidgetItem(['', dt_s, str(worker), where, msg[:-1]])

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
        #self.script_editor.open_resource(resource_name)
