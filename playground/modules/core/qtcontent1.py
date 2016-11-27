from PyQt5.QtWidgets import QMainWindow

from playground.ui.scripteditorwindow import Ui_MainWindow


class QtCcontent1(QMainWindow, Ui_MainWindow):
    def __init__(self, frontend):
        super(QtCcontent1, self).__init__()

        self.setupUi(self)

    def undo(self):
        pass

    def redo(self):
        pass

    def send(self):
        pass

    def save(self):
        pass
