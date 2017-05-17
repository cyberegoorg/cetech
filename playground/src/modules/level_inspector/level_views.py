import logging

import yaml
from PyQt5.QtWidgets import QMainWindow, QTreeWidgetItem

from cetech.playground.frontend import CETechWiget
from modules.level_inspector.ui.levelinspector import Ui_MainWindow


class LevelEditorView(CETechWiget):
    def __init__(self, frontend, parent=None):
        self.frontend = frontend
        self.rpc = frontend.rpc

        self.frontend.subscribe_service("asset_service",
                                        self._subcribe_asset_browser)

        super(LevelEditorView, self).__init__(self.rpc, "level_view",
                                              parent=parent)

        self.created = False

    def _subcribe_asset_browser(self, msg):
        msg_type = msg['msg_type']

        name = msg['name']
        type = msg['type']

        if msg_type == 'dclick' and type == 'level':
            self.rpc.call_service("engine_service", "call",
                                  instance_name=self.instance_name,
                                  fce_name="lua_execute",
                                  script="Editor:load_level(\"%s\")" % name)

    def showEvent(self, event):
        if not self.created:
            self.rpc.call_service("engine_service", "run_develop",
                                  name="level_view",
                                  compile_=True, continue_=True,
                                  lock=True, wid=int(self.wid),
                                  rpc_addr="ipc:///tmp/cetech_level_editor_rpc.ipc",
                                  log_addr="ipc:///tmp/cetech_level_editor_log.ipc",
                                  pub_addr="ipc:///tmp/cetech_level_editor_pub.ipc",
                                  push_addr="ipc:///tmp/cetech_level_editor_push.ipc",
                                  bootscript="playground/leveleditor_boot")

            self.created = True

        event.accept()

    def closeEvent(self, event):
        self.rpc.call_service("engine_service", "kill", name="level_view")
        event.accept()


class LevelInspectorView(QMainWindow, Ui_MainWindow):
    NAME = 0
    GUID = 1

    def __init__(self, frontend):
        self.frontend = frontend
        self.rpc = frontend.rpc
        self.path = None
        self.name = None

        self.project_service_rpc = self.rpc.partial_call_service("project_service")

        self.asset_service_rpc = self.rpc.partial_call_service("asset_service")
        self.filesystem_service_rpc = self.rpc.partial_call_service("filesystem_service")
        self.frontend_service_rpc = self.rpc.partial_call_service("frontend_service")

        super(LevelInspectorView, self).__init__()
        self.setupUi(self)

        self.frontend.subscribe_service("asset_service", self._subcribe_asset_browser)

    def entities_dclicked(self, item: QTreeWidgetItem, column: int):
        if item.parent() is not None:
            guid = item.text(self.GUID)

            self.frontend_service_rpc("publish",
                                      service_name="level_inspector",
                                      msg_type="event",
                                      msg=dict(type='entity_selected', guid=guid, level_name=self.name))

    def _subcribe_asset_browser(self, msg):
        msg_type = msg['msg_type']

        name = msg['name']
        type = msg['type']

        # TODO: dynamic
        path = "src/%s.%s" % (name, type)

        if msg_type == 'dclick' and type == 'level':
            self.path = path
            self.name = name

            content = self.filesystem_service_rpc("read", path=path)
            data = yaml.load(content)
            self._parse_data(data)

            self.asset_service_rpc = self.rpc.partial_call_service("asset_service")

    def _parse_data(self, data):
        self.entities_tree.clear()
        for group_name, group_entities in data.items():
            parent = self._add_groups(group_name)

            for entity_guid, entity_body in group_entities.items():
                ent_name = entity_body['name']

                self._add_entities(ent_name, entity_guid, parent)

        self.entities_tree.expandAll()

    def _add_groups(self, name):
        item = QTreeWidgetItem([name, ])
        self.entities_tree.addTopLevelItem(item)
        return item

    def _add_entities(self, name, entity_guid, parent):
        item = QTreeWidgetItem([name, entity_guid])
        parent.addChild(item)
        return item
