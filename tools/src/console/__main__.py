import sys
from PyQt5.QtWidgets import QApplication

from console.mainwindow import MainWindow

app = QApplication(sys.argv)
mw = MainWindow()
mw.show()

sys.exit(app.exec_())
