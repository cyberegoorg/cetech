from PyQt5 import QtCore
from PyQt5.QtCore import QSettings
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QListWidgetItem, QDialog

from cetech.playground.addexistingdialog import AddExistItemDialog
from cetech.playground.ui.projectmanagerdialog import Ui_Dialog


class ProjectManagerDialog(QDialog, Ui_Dialog):
    _PROJECT_DIR_ROLE = QtCore.Qt.UserRole + 1

    def __init__(self):
        super(ProjectManagerDialog, self).__init__()
        self.setupUi(self)

        self.exist_projects = []

    def add_exist(self, name, dir):
        self.exist_projects.append((name, dir))

        item = QListWidgetItem()
        item.setText(name)
        item.setData(self._PROJECT_DIR_ROLE, dir)
        item.setIcon(QIcon.fromTheme("document"))

        self.projects_list.addItem(item)

    def add_existing(self):
        dialog = AddExistItemDialog(self)
        if dialog.exec() != AddExistItemDialog.Accepted:
            return

        name, dir = dialog.get_project_settings()
        self.add_exist(name, dir)

    def remove_project(self):
        item = self.projects_list.takeItem(self.projects_list.selectedIndexes()[0].row())

        nd = (item.text(), item.data(self._PROJECT_DIR_ROLE))
        self.exist_projects.remove(nd)

    def project_list_dclicked(self, item):
        self.open_project_name = item.text()
        self.open_project_dir = item.data(self._PROJECT_DIR_ROLE)
        self.accept()

    def showEvent(self, event):
        s = QSettings()
        exist_projects = s.value("ProjectManager/exist_projects", []) or []

        for n, d in exist_projects:
            self.add_exist(n, d)

        event.accept()

    def done(self, r):
        s = QSettings()
        s.beginGroup("ProjectManager")
        s.setValue("exist_projects", self.exist_projects)
        s.endGroup()

        super(ProjectManagerDialog, self).done(r)
