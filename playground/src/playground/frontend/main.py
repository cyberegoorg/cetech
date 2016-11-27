import logging
import os
import sys

from PyQt5.QtWidgets import QApplication

from playground.frontend.private.app import FrontendApp

ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir, os.pardir)
MODULES_DIR = os.path.abspath(os.path.join(ROOT_DIR, "modules"))


def main():
    try:
        app = QApplication(sys.argv)
        app.setOrganizationName('cyberegoorg')
        app.setApplicationName('playground')

        pf = FrontendApp(MODULES_DIR)
        ret = app.exec_()
        pf.close()

    except Exception as e:
        logging.exception(e)
        sys.exit(-1)

    sys.exit(ret)
