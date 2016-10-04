import os

from PyQt5.QtCore import QTextCodec, QFile, QDir, QFileInfo, QUrl, pyqtSlot, Qt
from PyQt5.QtWebKit import QWebSettings
from PyQt5.QtWidgets import QFileDialog, QMessageBox, QDockWidget, QMainWindow

from playground.ui.scripteditorwindow import Ui_MainWindow

SUFIX_2_MODE = {
    'lua': 'ace/mode/lua',
    'sc': 'ace/mode/c_cpp',
    'sh': 'ace/mode/c_cpp',
    None: 'ace/mode/yaml'
}

SUPPORTED_EXT = ('lua', 'package', 'texture', 'json', 'unit', 'material', 'shader', 'sc', 'sh')
FILES_FILTER = "Lua (*.lua);;" \
               "Package (*.package);;" \
               "Texture (*.texture);;" \
               "JSON (*.json);;" \
               "Unit (*.unit);;" \
               "Material (*.material);;" \
               "Shader (*.shader)" \
               "Shader source (*.sc)" \
               "Shader header (*.sh)"


class ScriptEditorWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, project_manager, filename=None, parent=None):
        super(ScriptEditorWindow, self).__init__(parent=parent)
        self.setupUi(self)

        self.addAction(self.actionSave)
        self.addAction(self.actionSend)

        self.project_manager = project_manager

        self.modified = False
        self.filename = filename

        self.webView.page().settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)
        self.webView.page().mainFrame().setUrl(QUrl("file:///%s/html/editor.html" % QDir.currentPath()))

        self.webView.page().mainFrame().javaScriptWindowObjectCleared.connect(
            lambda: self.webView.page().mainFrame().addToJavaScriptWindowObject("editor_widget", self))

        if filename:
            self.webView.page().mainFrame().loadFinished.connect(lambda: self.open(self.filename))
        else:
            self.webView.page().mainFrame().loadFinished.connect(
                lambda: self.webView.page().mainFrame().evaluateJavaScript("init()"))

    @property
    def text(self):
        return self.webView.page().mainFrame().evaluateJavaScript("editor.getValue()")

    def open(self, filename):
        if not QFile.exists(filename):
            return

        file = QFile(filename)

        file.open(QFile.ReadOnly)
        data = file.readAll()
        codec = QTextCodec.codecForUtfText(data)
        unistr = codec.toUnicode(data)

        self._set_window_title(filename)

        self.webView.page().mainFrame().findFirstElement("#editor").setInnerXml(unistr)
        self.webView.page().mainFrame().evaluateJavaScript("init()")

        self._set_mode_by_filename(filename)

    def open_resource(self, resource_name):
        self.open(os.path.join(self.project_manager.source_dir, resource_name))

    def undo(self):
        self.webView.page().mainFrame().evaluateJavaScript("editor.undo()")

    def redo(self):
        self.webView.page().mainFrame().evaluateJavaScript("editor.redo()")

    def send(self):
        # TODO: IMPLEMENT self.api.lua_execute(self.text)
        pass

    def save(self):
        if self.filename is None:
            self._open_save_dialog()
            return

        self.save_as(self.filename)

    def save_as(self, filename):
        if not self.modified:
            return

        out_file = QFile(filename)
        out_file.open(QFile.WriteOnly)
        out_file.write(self.text.encode())
        out_file.close()

        self.modified = False
        self.filename = filename

        self._set_window_title(filename)

    def close_request(self):
        if self.filename is None:
            return False

        if self.modified:
            res = QMessageBox.question(self, "File is modified", "File '%s' is modified" % self.filename,
                                       QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel)
            if res == QMessageBox.Save:
                self.save()

            elif res == QMessageBox.Cancel:
                return False

        return True

    @pyqtSlot()
    def _on_change(self):
        self.modified = True

        title = self._get_window_title()

        if not title.endswith("*"):
            self._set_window_title("%s*" % title)

    def _set_mode_by_filename(self, filename):
        file = QFile(filename)
        fileinfo = QFileInfo(file)

        suffix = fileinfo.suffix()

        self.webView.page().mainFrame().evaluateJavaScript("editor.getSession().setMode('%s');" % (
            SUFIX_2_MODE[suffix] if suffix in SUFIX_2_MODE else SUFIX_2_MODE[None]
        ))

    def _set_window_title(self, filename):
        self.parent().setWindowTitle(filename.replace(self.project_manager.source_dir, '')[1:])

    def _get_window_title(self):
        return self.parent().windowTitle()

    def _set_window_title_modified(self, filename, modified):
        self.parent().setWindowTitle(filename.replace(self.project_manager.source_dir, '')[1:])

    def _open_save_dialog(self):
        idx = self.main_tabs.currentIndex()

        if not self.is_editor_modified():
            return

        filename, _ = QFileDialog.getSaveFileName(
            self,
            "Save file",
            QDir.currentPath() if self.project_manager.project_dir is None else os.path.join(
                self.project_manager.project_dir, 'src'), FILES_FILTER
        )

        if len(filename) == 0:
            return

        self.save_as(filename)


class ScriptEditorWidget(QDockWidget):
    def __init__(self, project_manager, filename=None):
        super(ScriptEditorWidget, self).__init__()

        self.editor = ScriptEditorWindow(project_manager=project_manager, filename=filename)
        self.setWidget(self.editor)

        self.setFeatures(QDockWidget.AllDockWidgetFeatures)
        self.setAttribute(Qt.WA_DeleteOnClose)

    @classmethod
    def support_ext(cls, ext):
        return ext in SUPPORTED_EXT

    def closeEvent(self, event):
        if self.editor.close_request():
            event.accept()
        else:
            event.ignore()


class ScriptEditorManager(object):
    def __init__(self, project_manager):
        self.project_manager = project_manager
        self.editors = {}

    def open(self, root, filename):
        if filename in self.editors:
            self.editors[filename].raise_()

        else:
            editor = ScriptEditorWidget(filename=filename, project_manager=self.project_manager)
            editor.setProperty("filename", filename)

            editor.destroyed.connect(self._editor_destroyed)

            root.addDockWidget(Qt.TopDockWidgetArea, editor)

            if len(self.editors):
                root.tabifyDockWidget(editor, tuple(self.editors.values())[0])

            self.editors[filename] = editor
            editor.raise_()

    def _editor_destroyed(self, obj):
        del self.editors[obj.property("filename")]
