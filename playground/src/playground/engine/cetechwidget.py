from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QWidget

from playground.engine.api import ConsoleProxy


class CetechWidget(QWidget):
    def __init__(self, parent, api: ConsoleProxy):
        self.api = api
        super(CetechWidget, self).__init__(parent, Qt.ForeignWindow)

    def resizeEvent(self, event):
        size = event.size()

        if self.api.connected:
            self.api.resize(size.width(), size.height())

        event.accept()
