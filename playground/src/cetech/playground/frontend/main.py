import logging
import os
import sys
from PyQt5.QtWidgets import QApplication

import modules as _modules
from cetech.playground.frontend import FrontendApp
from .projectmanagerdialog import ProjectManagerDialog

# SHITCODE
ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), *([os.pardir] * 4))
MODULES_DIR = os.path.abspath(os.path.dirname(_modules.__file__))
CORE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), *([os.pardir] * 5), "core"))


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
