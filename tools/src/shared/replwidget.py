from PyQt5.QtCore import QDir, QFile, QTextCodec
from PyQt5.QtWidgets import QFrame, QFileDialog
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

    def open_file(self):
        filename, _ =  QFileDialog.getOpenFileName(self, "Open .cpp file", QDir.currentPath(), "Lua (*.lua)")

        if not QFile.exists(filename):
            return

        file = QFile(filename)
        file.open(QFile.ReadOnly)

        data = file.readAll()
        codec = QTextCodec.codecForUtfText(data)
        unistr = codec.toUnicode(data)

        self.repl_edit.setText(unistr)
