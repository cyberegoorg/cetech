from PyQt5.QtWidgets import QFrame

from shared.ui.consolewidget import Ui_ConsoleWidget


class ConsoleWidget(QFrame, Ui_ConsoleWidget):
    LOG_COLOR_TAG = {
        'I': "%s",
        'W': "<span style='color: yellow;'> %s</span>",
        'D': "%s",
        'E': "<span style='color: red;'> %s</span>",
    }

    def __init__(self, api):
        super(ConsoleWidget, self).__init__()
        self.setupUi(self)

        self.api = api
        self.api.register_on_log_handler(self.add_log)


    def add_log(self, level, where, message):
        if 'lua' in where:
            self.command_log_textedit.append(self.LOG_COLOR_TAG[level] % message)

    def send_command(self):
        text = self.command_lineedit.text()

        if len(text) > 0:
            self.api.lua_execute(text)
            self.command_log_textedit.append("> %s" % text)

        self.command_lineedit.setText("")
