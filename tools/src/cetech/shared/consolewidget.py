from PyQt5.QtGui import QTextCursor
from PyQt5.QtWidgets import QFrame

from cetech.shared import Ui_ConsoleWidget


class ConsoleWidget(QFrame, Ui_ConsoleWidget):
    LOG_COLOR_TAG = {
        'I': "%s",
        'W': "<span style='color: yellow;'> %s </span>",
        'D': "%s",
        'E': "<span style='color: red;'> %s </span>",
    }

    def __init__(self, api):
        super(ConsoleWidget, self).__init__()
        self.setupUi(self)

        self.api = api
        self.api.register_handler('log', self.add_log)

    def add_log(self, time, level, where, msg):
        if 'lua' in where:
            self._add_line(self.LOG_COLOR_TAG[level] % msg)

    def _add_line(self, line):
        self.command_log_textedit.append(line)

        c =  self.command_log_textedit.textCursor()
        c.movePosition(QTextCursor.End)
        self.command_log_textedit.setTextCursor(c)
        self.command_log_textedit.ensureCursorVisible()

    def send_command(self):
        text = self.command_lineedit.text()

        if len(text) > 0:
            self.api.lua_execute(text)
            self._add_line("<span style='color: black;'> > %s </span>" % text)

        self.command_lineedit.setText("")
