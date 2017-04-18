from PyQt5.QtWidgets import QDialog, QListWidgetItem

from .ui.selectassetdialog import Ui_Dialog


class SelectAssetDialog(QDialog, Ui_Dialog):
    def __init__(self, name, asset_type, rpc):
        super(SelectAssetDialog, self).__init__()
        self.rpc = rpc

        self.setupUi(self)
        self.setWindowTitle("Select %s asset" % asset_type)

        self.asset_name_edit.setText(name)
        self.asset_name = name

        self.asset_service_rpc = rpc.partial_call_service("asset_service")
        self._fill_assets(asset_type)

        self.asset_list.setFocus()

    def asset_activated(self, item: QListWidgetItem):
        asset_name = item.text()
        self.asset_name_edit.setText(asset_name)

    def _fill_assets(self, asset_type):
        assets = self.asset_service_rpc("get_assets",
                                        asset_type=asset_type)

        curent = None
        for asset in assets:
            item = QListWidgetItem(asset)
            self.asset_list.addItem(item)

            if asset == self.asset_name:
                curent = item

        self.asset_list.setCurrentItem(curent)

    def done(self, r):
        if r == QDialog.Accepted:
            self.asset_name = self.asset_name_edit.text()

        super(SelectAssetDialog, self).done(r)
