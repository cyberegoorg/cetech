from PyQt5.QtCore import QDir
from PyQt5.QtWidgets import QMessageBox


def validate_project(project_dir):
    selected_dir = QDir(project_dir)

    if not selected_dir.exists():
        QMessageBox.critical(None, 'Dir does not  exist', 'Dir does not  exist',
                             QMessageBox.Yes, QMessageBox.Yes)
        return False

    if not selected_dir.exists('src/global.config'):
        QMessageBox.critical(None, 'Project validation error', 'Project dir does not contain src/global.config',
                             QMessageBox.Yes, QMessageBox.Yes)
        return False

    return True
