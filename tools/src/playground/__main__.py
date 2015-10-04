import sys
from PyQt5.QtWidgets import QApplication

from playground.mainwindow import MainWindow

app = QApplication(sys.argv)
mw = MainWindow()
mw.show()

sys.exit(app.exec_())
