import os

from PyQt5 import Qsci
from PyQt5.QtCore import QTextCodec, QFile, QDir, QFileInfo, QByteArray
from PyQt5.QtGui import QColor
from PyQt5.QtWidgets import QMainWindow, QFileDialog, QMessageBox
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

        idx = self.find_tab_by_filename(filename)
        if idx is not None:
            self.main_tabs.setCurrentIndex(idx)
            return

        file = QFile(filename)
        fileinfo = QFileInfo(file)

        file.open(QFile.ReadOnly)
        data = file.readAll()
        codec = QTextCodec.codecForUtfText(data)
        unistr = codec.toUnicode(data)

        idx, sci = self.create_new_editor(fileinfo.fileName())
        sci.setText(unistr)
        sci.setModified(False)
        sci.setProperty("filename", filename)

    def find_tab_by_filename(self, filename):
        for i in range(self.main_tabs.count()):
            if filename == self.get_editor_filename(i):
                return i

        return None

    def open_new_file(self):
        # TODO: Untitled + id (Untitled1, Untitled2, ... )
        idx, sci = self.create_new_editor("Untitled")
        sci.setProperty("filename", "")

    def create_new_editor(self, name):
        sci = Qsci.QsciScintilla(self)
        lexer = Qsci.QsciLexerLua()
        sci.setLexer(lexer)

        sci.setCaretLineVisible(True)
        sci.setCaretLineBackgroundColor(QColor("gray"))

        idx = self.main_tabs.addTab(sci, name)
        self.main_tabs.setCurrentIndex(idx)

        return idx, sci

    def save_as(self, tab_index, filename):
        if not self.is_editor_modified(tab_index):
            return

        sci = self.main_tabs.widget(tab_index)

        out_file = QFile(filename)
        out_file.open(QFile.WriteOnly)
        out_file.write(sci.text().encode())
        out_file.close()

        sci.setModified(False)

        sci.setProperty("filename", filename)

    def tab_close_request(self, tab_index):
        filename = self.get_editor_name(tab_index)

        if self.is_editor_modified(tab_index):
            res = QMessageBox.question(self, "File is modified", "File '%s' is modified" % filename, QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
            if res == QMessageBox.Save:
                self.save_file(tab_index)

        self.main_tabs.removeTab(tab_index)

    def get_editor_filename(self, tab_index):
        sci = self.main_tabs.widget(tab_index)
        return sci.property("filename")

    def get_editor_name(self, tab_index):
        return self.main_tabs.tabText(tab_index)

    def is_editor_modified(self, tab_index):
        sci = self.main_tabs.widget(tab_index)
        return sci.isModified()

    def save_file(self, tab_index):
        if self.get_editor_filename(tab_index) == "":
            self.open_save_dialog()
            return

        self.save_as(tab_index, self.get_editor_filename(tab_index))

    def save_current(self):
        idx = self.main_tabs.currentIndex()
        self.save_file(idx)

    def save_all(self):
        for i in range(self.main_tabs.count()):
            self.save_file(i)

    def open_file_dialog(self):
        filename, _ = QFileDialog.getOpenFileName(
            self,
            "Open lua file",
            QDir.currentPath() if self.project_manager.project_dir is None else os.path.join(
                self.project_manager.project_dir, 'src'),
            "Lua (*.lua)"
        )
        self.open_file(filename)

    def open_save_dialog(self):
        idx = self.main_tabs.currentIndex()

        if not self.is_editor_modified(idx):
            return

        filename, _ = QFileDialog.getSaveFileName(
            self,
            "Save file",
            QDir.currentPath() if self.project_manager.project_dir is None else os.path.join(
                self.project_manager.project_dir, 'src'),
            "Lua (*.lua)"
        )

        self.save_as(idx, filename)
