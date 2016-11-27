import sys

from PyQt5.QtWidgets import QApplication

from playground.backend import server
from playground.frontend.private.projectmanagerdialog import ProjectManagerDialog


def main():
    app = QApplication(sys.argv)
    app.setOrganizationName('cyberegoorg')
    app.setApplicationName('playground')

    pm = ProjectManagerDialog()
    ret = pm.exec()

    if ret == pm.Accepted:
        name, directory = pm.open_project_name, pm.open_project_dir

        server.run(name, directory)

    sys.exit(0)
