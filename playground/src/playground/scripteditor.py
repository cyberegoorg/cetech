import os

from PyQt5.QtCore import QTextCodec, QFile, QDir, QFileInfo, QUrl, pyqtSlot, Qt
from PyQt5.QtGui import QPixmap
from PyQt5.QtWebKit import QWebSettings
from PyQt5.QtWebKitWidgets import QWebView
from PyQt5.QtWidgets import QMainWindow, QFileDialog, QMessageBox, QDockWidget

from playground.ui.luaeditorwindow import Ui_MainWindow


# TODO: big shit code... refactor`em all

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

                self.open(filename)
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

    def open(self, filename):
        file = QFile(filename)

        file.open(QFile.ReadOnly)
        data = file.readAll()
        codec = QTextCodec.codecForUtfText(data)
        unistr = codec.toUnicode(data)

        self.page().mainFrame().findFirstElement("#editor").setInnerXml(unistr)

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

    def __init__(self, filename, project_manager, api):
        super(ScriptEditor, self).__init__()
        self.setupUi(self)

        self.project_manager = project_manager
        self.api = api

        self.editor_widget = ScriptEditorWidget(filename, self)
        self.setCentralWidget(self.editor_widget)

        if filename:
            self.open_file(filename)

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

    @classmethod
    def support_ext(cls, ext):
        return ext in cls.SUPPORTED_EXT

    def open_file(self, filename):
        if not QFile.exists(filename):
            return
        self.editor_widget.open(filename)

    def open_resource(self, resource_name):
        self.open_file(os.path.join(self.project_manager.source_dir, resource_name))

    def send_current(self):
        sci = self.main_tabs.widget(self.main_tabs.currentIndex())
        self.api.lua_execute(sci.text)

    def save_as(self, filename):
        if not self.is_editor_modified():
            return

        out_file = QFile(filename)
        out_file.open(QFile.WriteOnly)
        out_file.write(self.editor_widget.text.encode())
        out_file.close()

        self.editor_widget.modified = False
        self.editor_widget.filename = filename

    def close_request(self):
        if self.editor_widget.filename == '':
            return False

        if self.is_editor_modified():
            res = QMessageBox.question(self, "File is modified", "File '%s' is modified" % self.editor_widget.filename,
                                       QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
            if res == QMessageBox.Save:
                self.save_file()

            elif res == QMessageBox.Cancel:
                return False

        return True

    def get_editor_filename(self):
        return self.get_editor().filename

    def get_editor_name(self):
        return self.get_editor().filename

    def get_editor(self):
        """
        :rtype: ScriptEditorWidget
        """
        return self.editor_widget

    def is_editor_modified(self, ):
        return self.get_editor().modified

    def undo(self):
        self.get_editor().undo()

    def redo(self):
        self.get_editor().redo()

    def save_file(self):
        if self.get_editor_filename() == "":
            self.open_save_dialog()
            return

        self.save_as(self.get_editor_filename())

    def save_current(self):
        self.save_file()

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

        if not self.is_editor_modified():
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

        self.save_as(filename)


class ScriptEditorDock(QDockWidget):
    def __init__(self, filename, project_manager, api):
        super(ScriptEditorDock, self).__init__()

        self.script_editor = ScriptEditor(filename=filename, project_manager=project_manager, api=api)
        self.setWindowTitle("Script editor")
        self.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.setWidget(self.script_editor)
        self.setAttribute(Qt.WA_DeleteOnClose)

    def closeEvent(self, event):
        if self.script_editor.close_request():
            event.accept()

        else:
            event.ignore()
