from PyQt5.QtCore import QDir
from PyQt5.QtWidgets import QFileSystemModel
from PyQt5.QtWidgets import QMainWindow

import playground.core.rpc as rpc
from playground.core.widget import CETechWiget
from playground.ui.assetbrowser import Ui_MainWindow


class AssetPreviewView(CETechWiget):
    def __init__(self, frontend, parent=None):
        self.frontend = frontend

        self.rpc = rpc.Client(url="ws://localhost:8888", recv_timeout=10 * 1000)
        self.rpc.connect()

        self.frontend.subscribe_service("asset_service", self._subcribe_asset_browser)

        super(AssetPreviewView, self).__init__(self.rpc, "asset_preview", parent=parent)

        self.created = False

    def showEvent(self, event):
        if not self.created:
            self.rpc.call_service("engine_service", "run_develop",
                                  name="asset_preview",
                                  lock=True, wid=int(self.wid),
                                  rpc_addr="ipc:///tmp/cetech_asset_preview_rpc.ipc",
                                  log_addr="ipc:///tmp/cetech_asset_preview_log.ipc",
                                  pub_addr="ipc:///tmp/cetech_asset_preview_pub.ipc",
                                  push_addr="ipc:///tmp/cetech_asset_preview_push.ipc",
                                  bootscript="playground/assetview_boot")
            self.created = True

        event.accept()

    def _subcribe_asset_browser(self, msg):
        msg_type = msg['msg_type']
        name = msg['name']
        type = msg['type']

        if msg_type == 'click':
            self.rpc.call_service("engine_service", "call", instance_name=self.instance_name, fce_name="lua_execute",
                                  script="AssetView:show_asset(\"%s\", \"%s\")" % (name, type))

    def closeEvent(self, event):
        self.rpc.call_service("engine_service", "kill", name="asset_preview")
        event.accept()


class AssetBrowserView(QMainWindow, Ui_MainWindow):
    def __init__(self, frontend):
        self.rpc = rpc.Client(url="ws://localhost:8888", recv_timeout=10 * 1000)
        self.rpc.connect()

        self.project_service_rpc = self.rpc.partial_call_service("project_service")
        self.asset_service_rpc = self.rpc.partial_call_service("asset_service")

        super(AssetBrowserView, self).__init__()
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

    def showEvent(self, event):
        self._open_dir(self.project_service_rpc("get_project_dir"))
        event.accept()

    def closeEvent(self, event):
        event.accept()

    def _open_dir(self, path):
        self.project_dir = path

        self.dir_model.setRootPath(self.project_dir)
        self.file_model.setRootPath(self.project_dir)

        self.dir_view.setRootIndex(self.dir_model.index(self.project_dir))
        self.file_view.setRootIndex(self.file_model.index(self.project_dir))

    def dir_clicked(self, idx):
        path = self.dir_model.fileInfo(idx).absoluteFilePath()

        self.file_view.setRootIndex(self.file_model.setRootPath(path))

    def file_doubleclicked(self, idx):
        fileinfo = self.file_model.fileInfo(idx)

        path = fileinfo.absoluteFilePath()
        ext = fileinfo.suffix()
        asset_name = path.replace(self.project_dir, '').replace('/src/', '').split('.')[0]

        self.asset_service_rpc("asset_dclick", path=path, name=asset_name, type=ext)

    def file_clicked(self, idx):
        fileinfo = self.file_model.fileInfo(idx)

        path = fileinfo.absoluteFilePath()
        ext = fileinfo.suffix()
        asset_name = path.replace(self.project_dir, '').replace('/src/', '').split('.')[0]

        self.asset_service_rpc("asset_click", path=path, name=asset_name, type=ext)
