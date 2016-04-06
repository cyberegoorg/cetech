import nanomsg
import yaml
from PyQt5.QtCore import Qt, QThread
from PyQt5.QtWidgets import QWidget
from nanomsg import Socket, SUB, SUB_SUBSCRIBE


class ReadyLock(QThread):
    def __init__(self, url, engine_widget):
        self.socket = Socket(SUB)
        self.url = url
        self.engine_widget = engine_widget

        super().__init__()

    def register_handler(self, handler):
        self.handlers.append(handler)

    def run(self):
        self.socket.set_string_option(SUB, SUB_SUBSCRIBE, b'')
        self.socket.connect(self.url)
        while True:
            try:
                msg = self.socket.recv()
                msg_yaml = yaml.load(msg)

                if msg_yaml["where"] == 'application.ready':
                    size = self.engine_widget.size()
                    print("ready", size.width(), size.height())

                    self.engine_widget.ready = True
                    #self.api.resize(size.width(), size.height())
                    return

            except nanomsg.NanoMsgAPIError as e:
                raise


class CetechWidget(QWidget):
    def __init__(self, parent, api, log_url):
        self.api = api
        self.ready = False

        self.ready_lock = ReadyLock(log_url, self)
        self.ready_lock.start(QThread.NormalPriority)

        super(CetechWidget, self).__init__(parent, Qt.ForeignWindow)

    def set_api(self, api):
        self.api = api

    def resizeEvent(self, event):
        if self.api and self.ready:
            size = event.size()
            self.api.resize(size.width(), size.height())

        event.accept()
