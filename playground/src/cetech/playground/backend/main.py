import sys

from PyQt5.QtWidgets import QApplication

from cetech.playground.backend import server


def main():
    app = QApplication(sys.argv)
    app.setOrganizationName('cyberegoorg')
    app.setApplicationName('playground')

    server.run()

    sys.exit(0)
