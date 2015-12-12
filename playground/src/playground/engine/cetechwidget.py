from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QWidget


class CetechWidget(QWidget):
    def __init__(self, parent, api):
        self.api = api
        super(CetechWidget, self).__init__(parent, Qt.ForeignWindow)

    def resizeEvent(self, event):
        size = event.size()

        self.api.resize(size.width(), size.height())

        event.accept()
