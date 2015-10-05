import os

from PyQt5 import Qsci

from PyQt5.QtCore import QTextCodec, QFile, QDir, QFileInfo
from PyQt5.QtWidgets import QMainWindow, QFileDialog

from playground.projectmanager import ProjectManager
from shared.ui.luaeditorwindow import Ui_MainWindow


class LuaEditor(QMainWindow, Ui_MainWindow):
    def __init__(self, project_manager: ProjectManager):
        super(LuaEditor, self).__init__()
        self.setupUi(self)

        self.project_manager = project_manager

    def open_file(self, filename):
        if not QFile.exists(filename):
            return

        file = QFile(filename)
        fileinfo = QFileInfo(file)

        file.open(QFile.ReadOnly)
        data = file.readAll()
        codec = QTextCodec.codecForUtfText(data)
        unistr = codec.toUnicode(data)

        sci = self.create_new_tab(fileinfo.fileName())
        sci.setText(unistr)
        sci.setProperty("filename", filename)

    def create_new_tab(self, name):
        sci = Qsci.QsciScintilla(self)
        lexer = Qsci.QsciLexerLua()
        sci.setLexer(lexer)

        self.main_tabs.addTab(sci, name)
        return sci

    def tab_close_request(self, tab_index):
        sci = self.main_tabs.widget(tab_index)

        self.main_tabs.removeTab(tab_index)

    def open_file_dialog(self):
        filename, _ = QFileDialog.getOpenFileName(
            self,
            "Open lua file",
            os.path.join(self.project_manager.project_dir, 'src'),
            "Lua (*.lua)"
        )
        self.open_file(filename)
