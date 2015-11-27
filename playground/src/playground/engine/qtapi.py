from PyQt5.QtCore import QThread

from playground.engine.api import ConsoleAPI


class QtConsoleAPI(ConsoleAPI, QThread):
    def __init__(self, address, port):
        super(QtConsoleAPI, self).__init__(address, port, lambda: QThread.msleep(10) )

    def run(self):
        self.run_loop()
