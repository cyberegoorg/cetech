from PyQt5.QtCore import QThread

from cetech.api import ConsoleAPI


class QtConsoleAPI(ConsoleAPI, QThread):
    def __init__(self, address, port):
        super(QtConsoleAPI, self).__init__(address, port)

    def run(self):
        self.run_loop()
