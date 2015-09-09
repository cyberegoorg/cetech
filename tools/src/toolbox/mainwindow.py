from PyQt5.QtWidgets import QMainWindow, QFileDialog

from toolbox.ui.mainwindow import Ui_MainWindow


class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        self.setupUi(self)

    def select_project_dir(self):
        selected_dir = QFileDialog.getExistingDirectory(self, "Select Directory")
        self.ProjectDir_edit.setText(selected_dir)
