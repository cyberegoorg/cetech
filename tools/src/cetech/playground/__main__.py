import sys

from PyQt5.QtWidgets import QApplication

from cetech.playground.mainwindow import MainWindow
from cetech.playground.projectmanagerdialog import ProjectManagerDialog
from cetech.playground.rc.playground import qInitResources

qInitResources()

app = QApplication(sys.argv)
app.setOrganizationName('cyberegoorg')
app.setApplicationName('playground')

pm = ProjectManagerDialog()
ret = pm.exec()
if ret == pm.Accepted:
    name, dir = pm.open_project_name, pm.open_project_dir

    mw = MainWindow()
    mw.show()
    mw.focusWidget()
    mw.open_project(name, dir)

    sys.exit(app.exec_())

else:
    sys.exit(0)
