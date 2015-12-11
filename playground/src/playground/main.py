import sys

from PyQt5.QtWidgets import QApplication

from playground.mainwindow import MainWindow
from playground.projectmanagerdialog import ProjectManagerDialog


def main():
    app = QApplication(sys.argv)
    app.setOrganizationName('cyberegoorg')
    app.setApplicationName('playground')

    pm = ProjectManagerDialog()
    ret = pm.exec()

    if ret == pm.Accepted:
        name, directory = pm.open_project_name, pm.open_project_dir

        mw = MainWindow()
        mw.show()
        mw.focusWidget()
        mw.open_project(name, directory)

        sys.exit(app.exec_())

    else:
        sys.exit(0)
