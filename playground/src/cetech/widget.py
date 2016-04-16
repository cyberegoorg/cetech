import nanomsg
import yaml

import PyQt5.QtGui
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
                    self.engine_widget.ready = True
                    # self.api.resize(size.width(), size.height())
                    return

            except nanomsg.NanoMsgAPIError as e:
                raise

class Widget(QWidget):
    def __init__(self, parent, api, log_url):
        self.api = api
        self.ready = False
        self.last_pos = (0, 0)

        self.ready_lock = ReadyLock(log_url, self)
        self.ready_lock.start(QThread.NormalPriority)

        super(Widget, self).__init__(parent, Qt.ForeignWindow)

        #self.setAttribute(Qt.WA_TransparentForMouseEvents)

    def resizeEvent(self, event):
        if self.api and self.ready:
            size = event.size()
            self.api.resize(size.width(), size.height())

        event.accept()

    def _move_mouse(self, x, y, left, midle, right):
        self.api.send_command('lua.execute',
                              script="EditorMouse:Move(%d, %d, %s, %s, %s)" % (x, y, left, midle, right))

    def mouseMoveEvent(self, event):
        """ QWidget.mouseMoveEvent(QMouseEvent) 
        :type event: PyQt5.QtGui.QMouseEvent.QMouseEvent
        """

        if not self.ready:
            return

        buttons = event.buttons()

        left = str(buttons == Qt.LeftButton).lower()
        midle = str(buttons == Qt.MidButton).lower()
        right = str(buttons == Qt.RightButton).lower()

        last_pos = self.last_pos

        actual_pos = (event.pos().x(), event.pos().y())
        self._move_mouse(event.pos().x(), event.pos().y(), left, midle, right)
