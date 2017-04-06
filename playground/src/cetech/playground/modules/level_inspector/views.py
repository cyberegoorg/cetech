import yaml
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QStandardItemModel, QStandardItem
from PyQt5.QtWidgets import QMainWindow

from cetech.playground.modules.level_inspector.ui.levelinspector import \
    Ui_MainWindow


class LevelInspectorView(QMainWindow, Ui_MainWindow):
    NAME = 0

    def __init__(self, frontend):
        self.frontend = frontend
        self.rpc = frontend.rpc
        self.path = None

        self.project_service_rpc = self.rpc.partial_call_service(
            "project_service")

        self.asset_service_rpc = self.rpc.partial_call_service("asset_service")
        self.filesystem_service_rpc = self.rpc.partial_call_service(
            "filesystem_service")

        super(LevelInspectorView, self).__init__()
        self.setupUi(self)

        self.frontend.subscribe_service("asset_service",
                                        self._subcribe_asset_browser)

    def _subcribe_asset_browser(self, msg):
        msg_type = msg['msg_type']

        name = msg['name']
        type = msg['type']

        # TODO: dynamic
        path = "src/%s.%s" % (name, type)

        if msg_type == 'dclick' and type == 'level':
            self.path = path
            self.model = self._create_entities_model(self)
            self.entities_tree.setModel(self.model)

            content = self.filesystem_service_rpc("read", path=path)
            data = yaml.load(content)
            self._parse_data(data)

    def _parse_data(self, data):
        for group_name, group_entities in data.items():
            parent = self._add_groups(self.model, group_name)

            for entity_guid, entity_body in group_entities.items():
                ent_name = entity_body['name']

                self._add_entities(self.model,
                                   "%s (%s)" % (ent_name, entity_guid), parent)

        self.entities_tree.expandAll()


    def _create_entities_model(self, parent):
        model = QStandardItemModel(0, 1, parent)
        model.setHeaderData(self.NAME, Qt.Horizontal, "Name")

        return model

    def _add_groups(self, model, name):
        row = QStandardItem(name)
        model.appendRow(row)
        return row

    def _add_entities(self, model, name, parent):
        row = QStandardItem(name)
        parent.appendRow(row)
        return row
