from PyQt5.QtWidgets import QMainWindow

from shared.ui.luaeditorwindow import Ui_MainWindow


class LuaEditor(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(LuaEditor, self).__init__()
        self.setupUi(self)

