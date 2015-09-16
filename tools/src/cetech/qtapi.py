from PyQt5.QtCore import QTimer, QThread
from cetech.api import ConsoleAPI


class QtConsoleAPI(ConsoleAPI, QThread):
    def __init__(self, address, port):
        super(QtConsoleAPI, self).__init__(address, port)

        #self.timer = QTimer()
        #self.timer.timeout.connect(self.tick)

    def start_tick(self, period):
        pass#self.timer.start(period)

