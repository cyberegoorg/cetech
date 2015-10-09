import os

from PyQt5.QtCore import QDir, pyqtSignal
from PyQt5.QtWidgets import QMainWindow, QFileSystemModel

from cetech.playground.ui.assetbrowser import Ui_MainWindow


class AssetBrowser(QMainWindow, Ui_MainWindow):
    asset_clicked = pyqtSignal(str, str, name='assetClicked')

    def __init__(self):
        super(AssetBrowser, self).__init__()
        self.setupUi(self)

        self.dir_model = QFileSystemModel()
        self.dir_model.setFilter(QDir.NoDotAndDotDot | QDir.AllDirs)
        self.dir_model.setReadOnly(False)

        self.dir_view.setModel(self.dir_model)
        self.dir_view.hideColumn(1)
        self.dir_view.hideColumn(2)
        self.dir_view.hideColumn(3)

        self.file_model = QFileSystemModel()
        self.file_model.setFilter(QDir.NoDotAndDotDot | QDir.Files)
        self.file_model.setReadOnly(False)

        self.file_view.setModel(self.file_model)

    def open_project(self, project_dir):
        path = os.path.join(project_dir, 'src')

        self.dir_model.setRootPath(path)
        self.file_model.setRootPath(path)

        self.dir_view.setRootIndex(self.dir_model.index(path))
        self.file_view.setRootIndex(self.file_model.index(path))

    def dir_clicked(self, idx):
        path = self.dir_model.fileInfo(idx).absoluteFilePath()

        self.file_view.setRootIndex(self.file_model.setRootPath(path))

    def file_doubleclicked(self, idx):
        fileinfo = self.file_model.fileInfo(idx)

        path = fileinfo.absoluteFilePath()
        ext = fileinfo.suffix()

        self.asset_clicked.emit(path, ext)
