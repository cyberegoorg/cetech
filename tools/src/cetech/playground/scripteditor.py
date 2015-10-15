import os

from PyQt5 import Qsci
from PyQt5.QtCore import QTextCodec, QFile, QDir, QFileInfo
from PyQt5.QtGui import QColor, QPixmap, QKeySequence
from PyQt5.QtWidgets import QMainWindow, QFileDialog, QMessageBox, QShortcut

from cetech.engine.api import ConsoleAPI
from cetech.playground.cetechproject import CetechProject
from cetech.playground.qt.ui.luaeditorwindow import Ui_MainWindow


class ScriptEditor(QMainWindow, Ui_MainWindow):
    SUPPORTED_EXT = ('lua', 'package', 'json')
    FILES_FILTER = "Lua (*.lua); Package (*.package); JSON (*.json)"

    def __init__(self, project_manager: CetechProject, api: ConsoleAPI):
        super(ScriptEditor, self).__init__()
        self.setupUi(self)

        self.project_manager = project_manager
        self.api = api

        self.lexer = Qsci.QsciLexerLua()
        self.lua_api = Qsci.QsciAPIs(self.lexer)
        self.lua_api.prepare()
        self.lexer.setAPIs(self.lua_api)

        self.api.register_handler('autocomplete_list', self.update_autocomplete)

        self.autocomplete_shortcut = QShortcut(QKeySequence("Ctrl+Space"), self)
        self.autocomplete_shortcut.activated.connect(self.autocomplete)

        self.TYPE_ICONS = {
            0: QPixmap(":code-function"),
            1: QPixmap(":code-table"),
        }

    TYPE_TO_ICON_ID = {
        "function": 0,
        "table": 1
    }

    def autocomplete(self):
        self.get_editor(self.main_tabs.currentIndex()).autoCompleteFromAll()

    def update_autocomplete(self, list):
        self.lua_api.clear()
        for k, v in list.items():
            icon_id = self.TYPE_TO_ICON_ID.get(v, None)
            if icon_id is not None:
                self.lua_api.add("%s?%s" % (k, icon_id))

        self.lua_api.prepare()

    def support_ext(self, ext):
        return ext in self.SUPPORTED_EXT

    def open_file(self, filename):
        if not QFile.exists(filename):
            return

        # is file open?
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

        self.api.autocomplete_list()

    def find_tab_by_filename(self, filename):
        for i in range(self.main_tabs.count()):
            if filename == self.get_editor_filename(i):
                return i

        return None

    def close_all(self):
        while self.tab_close_request(0):
            pass

    def close_curent(self):
        self.tab_close_request(self.main_tabs.currentIndex())

    def send_current(self):
        sci = self.main_tabs.widget(self.main_tabs.currentIndex())
        self.api.lua_execute(sci.text())

    def open_new_file(self):
        # TODO: Untitled + id (Untitled1, Untitled2, ... )
        idx, sci = self.create_new_editor("Untitled")
        sci.setProperty("filename", "")

    def create_new_editor(self, name):
        sci = Qsci.QsciScintilla(self)
        sci.setLexer(self.lexer)

        sci.setCaretLineVisible(True)
        sci.setCaretLineBackgroundColor(QColor("gray"))

        sci.setAutoCompletionThreshold(1)
        sci.setAutoCompletionSource(Qsci.QsciScintilla.AcsAPIs)
        sci.setAutoCompletionFillupsEnabled(True)
        sci.setAutoCompletionShowSingle(True)

        for id, img in self.TYPE_ICONS.items():
            sci.registerImage(id, img)

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

        file_info = QFileInfo(out_file)
        self.main_tabs.setTabText(tab_index, file_info.fileName())

    def tab_close_request(self, tab_index):
        filename = self.get_editor_name(tab_index)
        if filename == '':
            return False

        if self.is_editor_modified(tab_index):
            res = QMessageBox.question(self, "File is modified", "File '%s' is modified" % filename,
                                       QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
            if res == QMessageBox.Save:
                self.save_file(tab_index)

            elif res == QMessageBox.Cancel:
                return False

        self.main_tabs.removeTab(tab_index)
        return True

    def get_editor_filename(self, tab_index):
        sci = self.main_tabs.widget(tab_index)
        return sci.property("filename")

    def get_editor_name(self, tab_index):
        return self.main_tabs.tabText(tab_index)

    def get_editor(self, tab_index):
        """
        :rtype: Qsci.QsciScintilla
        """
        return self.main_tabs.widget(tab_index)

    def is_editor_modified(self, tab_index):
        sci = self.main_tabs.widget(tab_index)
        return sci.isModified()

    def undo(self):
        sci = self.main_tabs.widget(self.main_tabs.currentIndex())
        sci.undo()

    def redo(self):
        sci = self.main_tabs.widget(self.main_tabs.currentIndex())
        sci.redo()

    def copy(self):
        sci = self.main_tabs.widget(self.main_tabs.currentIndex())
        sci.copy()

    def cut(self):
        sci = self.main_tabs.widget(self.main_tabs.currentIndex())
        sci.cut()

    def paste(self):
        sci = self.main_tabs.widget(self.main_tabs.currentIndex())
        sci.paste()

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
            "Open file",
            QDir.currentPath() if self.project_manager.project_dir is None else os.path.join(
                self.project_manager.project_dir, 'src'),
            self.FILES_FILTER
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
            self.FILES_FILTER
        )

        self.save_as(idx, filename)
