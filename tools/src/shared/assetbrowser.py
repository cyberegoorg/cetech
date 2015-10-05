import os

from PyQt5.QtCore import QDir, pyqtSignal, QFileInfo
from PyQt5.QtWidgets import QMainWindow, QFileSystemModel
from shared.ui.assetbrowser import Ui_MainWindow


class AssetBrowser(QMainWindow, Ui_MainWindow):
    asset_clicked = pyqtSignal(str, str, name='assetClicked')

    def __init__(self):
        super(AssetBrowser, self).__init__()
        self.setupUi(self)

        self.dir_model = QFileSystemModel()
        self.dir_model.setFilter(QDir.NoDotAndDotDot | QDir.AllDirs)
        self.treeView.setModel(self.dir_model)
        self.treeView.hideColumn(1)
        self.treeView.hideColumn(2)
        self.treeView.hideColumn(3)

        self.file_model = QFileSystemModel()
        self.listView.setModel(self.file_model)

    def open_project(self, project_dir):
        path = os.path.join(project_dir, 'src')

        self.dir_model.setRootPath(path)
        self.file_model.setRootPath(path)

        self.treeView.setRootIndex(self.dir_model.index(path))
        self.listView.setRootIndex(self.file_model.index(path))

    def dir_clicked(self, idx):
        path = self.dir_model.fileInfo(idx).absoluteFilePath()

        self.listView.setRootIndex(self.file_model.setRootPath(path))

    def file_doubleclicked(self, idx):
        fileinfo = self.file_model.fileInfo(idx)

        if fileinfo.isDir():
            return

        path = fileinfo.absoluteFilePath()
        ext = fileinfo.suffix()

        self.asset_clicked.emit(path, ext)