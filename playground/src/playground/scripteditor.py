import os
from PyQt5.QtCore import QTextCodec, QFile, QDir, QFileInfo, QUrl, pyqtSlot
from PyQt5.QtGui import QPixmap
from PyQt5.QtWebKit import QWebSettings
from PyQt5.QtWebKitWidgets import QWebView
from PyQt5.QtWidgets import QMainWindow, QFileDialog, QMessageBox
from playground.engine.api import ConsoleAPI
from playground.ui.luaeditorwindow import Ui_MainWindow
from playground.engine.cetechproject import CetechProject


class ScriptEditorWidget(QWebView):
    SUFIX_2_MODE = {
        'lua': 'ace/mode/lua',
        None: 'ace/mode/yaml'
    }

    def __init__(self, filename, parent=None):
        super(ScriptEditorWidget, self).__init__(parent)

        self.modified = False
        self.filename = filename

        self.page().settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)
        self.page().mainFrame().setUrl(QUrl("file://%s/html/editor.html" % QDir.currentPath()))
        # self.page().mainFrame().setHtml("html/editor.html", QDir.currentPath())

        if filename:
            def on_load():
                file = QFile(filename)
                fileinfo = QFileInfo(file)

                file.open(QFile.ReadOnly)
                data = file.readAll()
                codec = QTextCodec.codecForUtfText(data)
                unistr = codec.toUnicode(data)

                self.page().mainFrame().findFirstElement("#editor").setInnerXml(unistr)
                self.page().mainFrame().evaluateJavaScript("init()")

                suffix = fileinfo.suffix()
                self.page().mainFrame().evaluateJavaScript("editor.getSession().setMode('%s');" % (
                    self.SUFIX_2_MODE[suffix] if suffix in self.SUFIX_2_MODE else self.SUFIX_2_MODE[None]
                ))

            self.page().mainFrame().loadFinished.connect(on_load)
        else:
            self.page().mainFrame().loadFinished.connect(
                lambda: self.page().mainFrame().evaluateJavaScript("init()"))

        def create_editor_widget():
            self.page().mainFrame().addToJavaScriptWindowObject("editor_widget", self)

        self.page().mainFrame().javaScriptWindowObjectCleared.connect(create_editor_widget)

    @property
    def text(self):
        return self.page().mainFrame().evaluateJavaScript("editor.getValue()")

    @pyqtSlot()
    def _on_change(self):
        self.modified = True

    def undo(self):
        self.page().mainFrame().evaluateJavaScript("editor.undo()")

    def redo(self):
        self.page().mainFrame().evaluateJavaScript("editor.redo()")

    def set_mode_by_filename(self, filename):
        file = QFile(filename)
        fileinfo = QFileInfo(file)

        suffix = fileinfo.suffix()
        self.page().mainFrame().evaluateJavaScript("editor.getSession().setMode('%s');" % (
            self.SUFIX_2_MODE[suffix] if suffix in self.SUFIX_2_MODE else self.SUFIX_2_MODE[None]
        ))


class ScriptEditor(QMainWindow, Ui_MainWindow):
    SUPPORTED_EXT = ('lua', 'package', 'texture', 'json')
    FILES_FILTER = "Lua (*.lua);;Package (*.package);;Texture (*.texture);;JSON (*.json)"

    def __init__(self, project_manager: CetechProject, api: ConsoleAPI):
        super(ScriptEditor, self).__init__()
        self.setupUi(self)

        self.project_manager = project_manager
        self.api = api

        # self.api.register_handler('autocomplete_list', self.update_autocomplete)

        self.TYPE_ICONS = {
            0: QPixmap(":code-function"),
            1: QPixmap(":code-table"),
        }

    TYPE_TO_ICON_ID = {
        "function": 0,
        "table": 1
    }

    def update_autocomplete(self, list):
        pass
        # self.lua_api.clear()
        # self.lua_api.prepare()

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

        self.create_new_editor(fileinfo.fileName(), filename)

    def open_resource(self, resource_name):
        self.open_file(os.path.join(self.project_manager.source_dir, resource_name))

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
        self.api.lua_execute(sci.text)

    def open_new_file(self):
        # TODO: Untitled + id (Untitled1, Untitled2, ... )

        idx, sci = self.create_new_editor("Untitled")
        sci.setProperty("filename", "")

    def create_new_editor(self, name, filename=None):
        widget = ScriptEditorWidget(filename, self)

        idx = self.main_tabs.addTab(widget, name)
        self.main_tabs.setCurrentIndex(idx)

        return idx, widget

    def save_as(self, tab_index, filename):
        if not self.is_editor_modified(tab_index):
            return

        sci = self.get_editor(tab_index)
        sci.set_mode_by_filename(filename)

        out_file = QFile(filename)
        out_file.open(QFile.WriteOnly)
        out_file.write(sci.text.encode())
        out_file.close()

        sci.modified = False
        sci.filename = filename

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
        return self.get_editor(tab_index).filename

    def get_editor_name(self, tab_index):
        return self.main_tabs.tabText(tab_index)

    def get_editor(self, tab_index):
        """
        :rtype: ScriptEditorWidget
        """
        return self.main_tabs.widget(tab_index)

    def is_editor_modified(self, tab_index):
        return self.get_editor(tab_index).modified

    def undo(self):
        self.get_editor(self.main_tabs.currentIndex()).undo()

    def redo(self):
        self.get_editor(self.main_tabs.currentIndex()).redo()

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

        if len(filename) == 0:
            return

        self.save_as(idx, filename)
