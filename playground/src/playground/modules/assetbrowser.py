import os

from PyQt5.QtCore import QDir, pyqtSignal
from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QMainWindow, QFileSystemModel

from playground.core.modules import PlaygroundModule
from playground.ui.assetbrowser import Ui_MainWindow


class AssetBrowser(QMainWindow, Ui_MainWindow, PlaygroundModule):
    def __init__(self, module_manager):
        super(AssetBrowser, self).__init__()
        self.setupUi(self)

        self.init_module(module_manager)

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

        self.modules_manager.new_docked(self, "asset_browser", "Asset browser",
                                        Qt.BottomDockWidgetArea)

        self.modules_manager.main_window.splitDockWidget(self.modules_manager["asset_browser"].dock,
                                                         self.modules_manager["asset_view"].dock, Qt.Horizontal)

    def open_project(self, project):
        self.project = project
        project_dir = project.project_dir
        path = os.path.join(project_dir)

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
        asset_name = path.replace(self.project.project_dir, '').replace('/src/', '').split('.')[0]

        self.modules_manager.open_asset(path, asset_name, ext)

    def file_clicked(self, idx):
        fileinfo = self.file_model.fileInfo(idx)

        path = fileinfo.absoluteFilePath()
        ext = fileinfo.suffix()
        asset_name = path.replace(self.project.project_dir, '').replace('/src/', '').split('.')[0]

        self.modules_manager.show_asset(path, asset_name, ext)
