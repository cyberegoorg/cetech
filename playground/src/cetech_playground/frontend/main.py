import logging
import os
import sys

from PyQt5.QtWidgets import QApplication

from cetech_playground.frontend import FrontendApp
from cetech_playground.frontend import ProjectManagerDialog

ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir, os.pardir)
MODULES_DIR = os.path.abspath(os.path.join(ROOT_DIR, 'src', 'cetech_playground_modules'))
CORE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, os.pardir, os.pardir, "core"))

def main():
    try:
        app = QApplication(sys.argv)
        app.setOrganizationName('cyberegoorg')
        app.setApplicationName('playground')

        pm = ProjectManagerDialog()
        ret = pm.exec()

        if ret == pm.Accepted:
            name, directory = pm.open_project_name, pm.open_project_dir

            pf = FrontendApp(MODULES_DIR, name, directory, CORE_DIR)

            ret = app.exec_()
            pf.close()

        sys.exit(0)

    except Exception as e:
        logging.exception(e)
        sys.exit(-1)
