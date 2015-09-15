from PyQt5.QtWidgets import QFrame
from PyQt5 import Qsci

from shared.ui.replwidget import Ui_ReplWidget


class REPLWidget(QFrame, Ui_ReplWidget):
    def __init__(self, api):
        super(REPLWidget, self).__init__()
        self.setupUi(self)

        self.api = api
        #self.api.register_on_log_handler(self.add_log)

        lexer = Qsci.QsciLexerLua()
        self.repl_edit.setLexer(lexer)


    def add_log(self, level, where, message):
        if 'lua' in where:
            pass
            #self.command_log_textedit.append(self.LOG_COLOR_TAG[level] % message)

    def send_commands(self):
        text = self.repl_edit.text()

        if len(text) > 0:
            self.api.lua_execute(text)
            #self.command_log_textedit.append("> %s" % text)

        #self.command_lineedit.setText("")
