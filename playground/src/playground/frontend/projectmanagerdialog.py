from PyQt5 import QtCore
from PyQt5.QtCore import QSettings
from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QListWidgetItem, QDialog, QDialogButtonBox

from playground.frontend import AddExistItemDialog
from playground.ui.projectmanagerdialog import Ui_Dialog


class ProjectManagerDialog(QDialog, Ui_Dialog):
    _PROJECT_DIR_ROLE = QtCore.Qt.UserRole + 1

    def __init__(self):
        self.open_project_name = None
        self.open_project_dir = None
        self.exist_projects = []

        super(ProjectManagerDialog, self).__init__()
        self.setupUi(self)
        self.buttonBox.button(QDialogButtonBox.Open).setDisabled(True)

    def add_exist(self, name, dir):
        self.exist_projects.append((name, dir))

        item = QListWidgetItem()
        item.setText(name)
        item.setData(self._PROJECT_DIR_ROLE, dir)
        item.setIcon(QIcon().fromTheme("applications-games"))

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
        self.done(QDialog.Accepted)

    def project_selected(self):
        self.buttonBox.button(QDialogButtonBox.Open).setDisabled(False)

    def showEvent(self, event):
        s = QSettings()
        exist_projects = s.value("ProjectManager/exist_projects", []) or []

        for n, d in exist_projects:
            self.add_exist(n, d)

        event.accept()

    def done(self, r):
        if r == QDialog.Accepted:
            selected_project = self.projects_list.selectedItems()

            if len(selected_project) == 0:
                return

            selected_project = selected_project[0]

            self.open_project_name = selected_project.text()
            self.open_project_dir = selected_project.data(self._PROJECT_DIR_ROLE)

            s = QSettings()
            s.beginGroup("ProjectManager")
            s.setValue("exist_projects", self.exist_projects)
            s.endGroup()
            super(ProjectManagerDialog, self).done(r)

        super(ProjectManagerDialog, self).done(r)
