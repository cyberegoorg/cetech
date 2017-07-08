import logging
import os
import sys

import yaml
from PyQt5.QtGui import QPalette, QColor
from PyQt5.QtWidgets import QMainWindow, QTreeWidgetItem, QDoubleSpinBox, QHBoxLayout, QFrame, QLineEdit, QPushButton

import modules.property_inspector as _module
from modules.assets.select_asset_dialog import SelectAssetDialog
from modules.property_inspector.ui.propertyinspector import Ui_MainWindow

with open(os.path.join(os.path.dirname(os.path.abspath(_module.__file__)), "typedef.yaml"), "r") as f:
    dd_yml = f.read()


class TypeConstructor(object):
    def __init__(self, rpc):
        self.describe = yaml.load(dd_yml)
        self.rpc = rpc

        self.has = dict()
        self.name = dict()
        self.properties = dict()
        self.properties_set = dict()
        self.properties_get = dict()
        self.properties_ordered = dict()

        for component_type, component_body in self.describe['component'].items():
            self._build_component(component_type, component_body)

        self.types = self.has.keys()

    def _build_component(self, component_type, component_body):
        self.has[component_type] = self._buid_has(component_body['lua_name'])
        self.name[component_type] = component_body['name']

        self.properties[component_type] = component_body['properties']

        set_d = dict()
        get_d = dict()

        properties_ordered = []
        properties_sorted = sorted(component_body['properties'].items(), key=lambda x: x[1].get('order', 0))

        for prop_name, prop_body in properties_sorted:
            _type = prop_body["type"]

            if _type == 'vec3f':
                set_d[prop_name] = self._buid_set_vec3(component_type, prop_name)
                get_d[prop_name] = self._buid_get_vec3(component_type, prop_name)

            elif _type == 'string':
                set_d[prop_name] = self._buid_set_string(component_type, prop_name)
                get_d[prop_name] = self._buid_get_string(component_type, prop_name)

            elif _type == 'asset':
                set_d[prop_name] = self._buid_set_string(component_type, prop_name)
                get_d[prop_name] = self._buid_get_string(component_type, prop_name)

            properties_ordered.append(prop_name)

        self.properties_ordered[component_type] = properties_ordered
        self.properties_set[component_type] = set_d
        self.properties_get[component_type] = get_d

    def _buid_has(self, component_type):
        fce_body = """
            return (function (world, uid)
                local ent = Level.entity_by_id(Editor.level, uid)
                return %s.has(world, ent)
            end)(Editor.world, '%%s')
            """ % component_type

        def _(guid):
            return self.rpc.call_service(
                "engine_service", "call", instance_name="level_view", fce_name="lua_execute",
                script=fce_body % guid)

        return _

    def _buid_set_vec3(self, component_type, prop_name):
        fce_body = """
            return (function (world, uid, key, x, y, z)
                local ent = Level.entity_by_id(Editor.level, uid)
                Component.set_property(world, ent, "%s", key, Vec3f.make(x, y, z))
            end)(Editor.world, '%%s', '%s', %%f, %%f, %%f)
            """ % (component_type, prop_name)

        def _(guid, value):
            logging.info("Set prop %s.%s = %s", component_type, prop_name, value)

            return self.rpc.call_service(
                "engine_service", "call", instance_name="level_view", fce_name="lua_execute",
                script=fce_body % (guid, value[0], value[1], value[2]))

        return _

    def _buid_get_vec3(self, component_type, prop_name):
        fce_body = """
            return (function (world, uid, key)
                local ent = Level.entity_by_id(Editor.level, uid)
                local v = Component.get_property(world, ent, "%s", key)
                return {x=v.x, y=v.y, z=v.z}
            end)(Editor.world, '%%s', '%s')
            """ % (component_type, prop_name)

        def _(guid):
            ret = self.rpc.call_service(
                "engine_service", "call", instance_name="level_view", fce_name="lua_execute",
                script=fce_body % guid)
            return ret['x'], ret['y'], ret['z']

        return _

    def _buid_set_string(self, component_type, prop_name):
        fce_body = """
            return (function (world, uid, key, value)
                local ent = Level.entity_by_id(Editor.level, uid)
                Component.set_property(world, ent, "%s", key, value)
            end)(Editor.world, '%%s', '%s', '%%s')
            """ % (component_type, prop_name)

        def _(guid, value):
            logging.info("Set prop %s.%s = %s", component_type, prop_name, value)

            return self.rpc.call_service(
                "engine_service", "call", instance_name="level_view", fce_name="lua_execute",
                script=fce_body % (guid, value)
            )

        return _

    def _buid_get_string(self, component_type, prop_name):
        fce_body = """
            return (function (world, uid, key)
                local ent = Level.entity_by_id(Editor.level, uid)
                local v = Component.get_property(world, ent, "%s", key)
                return v
            end)(Editor.world, '%%s', '%s')
            """ % (component_type, prop_name)

        def _(guid):
            ret = self.rpc.call_service(
                "engine_service", "call", instance_name="level_view", fce_name="lua_execute",
                script=fce_body % guid)
            return ret

        return _


class PropertyInspectorView(QMainWindow, Ui_MainWindow):
    NAME = 0

    def __init__(self, frontend):
        self.frontend = frontend
        self.rpc = frontend.rpc
        self.path = None

        self.project_service_rpc = self.rpc.partial_call_service(
            "project_service")

        super(PropertyInspectorView, self).__init__()
        self.setupUi(self)

        self.frontend.subscribe_service("level_inspector",
                                        self._subcribe_level_inspector)

        self.tc = TypeConstructor(self.rpc)

    def _create_string(self, parent, guid, component_type, prop_name, prop_body, value):
        get_fce = self.tc.properties_get[component_type][prop_name]
        v = get_fce(guid)
        print(v)

        line_edit = QLineEdit()
        line_edit.setText(value if v is None else v)
        self.property_tree.setItemWidget(parent, 1, line_edit)

    def _create_asset(self, parent, guid, component_type, prop_name, prop_body, value):
        asset_type = prop_body["asset_type"]

        get_fce = self.tc.properties_get[component_type][prop_name]
        v = get_fce(guid)
        print(v)

        btn = QPushButton()
        btn.setText(value if v is None else v)

        def _open_select_asset_dialog():
            dialog = SelectAssetDialog(btn.text(), asset_type, self.rpc)

            if dialog.exec() != SelectAssetDialog.Accepted:
                return

            set_fce = self.tc.properties_set[component_type][prop_name]
            set_fce(guid, dialog.asset_name)
            btn.setText(dialog.asset_name)

        btn.clicked.connect(_open_select_asset_dialog)

        self.property_tree.setItemWidget(parent, 1, btn)

    def _create_vec3f(self, parent, guid, component_type, prop_name, prop_body, value):
        element_box = QHBoxLayout()
        frame = QFrame()
        frame.setLayout(element_box)

        def _create_spin(color, max, min):
            spin = QDoubleSpinBox()

            palette = spin.palette()
            palette.setColor(QPalette.Base, QColor(color))
            spin.setPalette(palette)

            spin.setMaximum(max)
            spin.setMinimum(min)
            spin.setWrapping(True)

            return spin

        min_spin = prop_body['min'] if 'min' in prop_body else (-sys.maxsize, ) * 3
        max_spin = prop_body['max'] if 'max' in prop_body else (sys.maxsize, ) * 3

        x_spin = _create_spin("red", max_spin[0], min_spin[0])
        y_spin = _create_spin("green", max_spin[1], min_spin[1])
        z_spin = _create_spin("blue", max_spin[2], min_spin[2])

        # get_fce = self.tc.properties_get[component_type][prop_name]

        # v = get_fce(guid)

        get_fce = self.tc.properties_get[component_type][prop_name]
        v = get_fce(guid)

        x_spin.setValue(v[0])
        y_spin.setValue(v[1])
        z_spin.setValue(v[2])

        set_fce = self.tc.properties_set[component_type][prop_name]

        def _on_x_value_change(value):
            v = (value, y_spin.value(), z_spin.value())
            set_fce(guid, v)

        def _on_y_value_change(value):
            v = (x_spin.value(), value, z_spin.value())
            set_fce(guid, v)

        def _on_z_value_change(value):
            v = (x_spin.value(), y_spin.value(), value)
            set_fce(guid, v)

        x_spin.valueChanged.connect(_on_x_value_change)
        y_spin.valueChanged.connect(_on_y_value_change)
        z_spin.valueChanged.connect(_on_z_value_change)

        element_box.addWidget(x_spin)
        element_box.addWidget(y_spin)
        element_box.addWidget(z_spin)

        self.property_tree.setItemWidget(parent, 1, frame)

    def _load_entity_def(self, level_name, guid):
        f = self.rpc.call_service("filesystem_service", "read", path="src/%s.level" % level_name)
        level_def = yaml.load(f)

        def _yaml_merge(root, parent):
            for k, v in parent.items():
                if k not in root:
                    root[k] = v
                elif isinstance(v, dict):
                    _yaml_merge(root[k], v)

        def _preprocess(ent_def, root):
            if 'prefab' in ent_def:
                prefab_f = self.rpc.call_service("filesystem_service", "read", path="src/%s.entity" % ent_def['prefab'])
                prefab_def = yaml.load(prefab_f)

                _preprocess(prefab_def, root)
                _yaml_merge(root, prefab_def)

        final = {}
        ent_def = level_def['entities'][guid]
        _preprocess(ent_def, final)

        return final

    def _dif_entity_def(self, level_name, guid, curent):
        level_def = self._load_entity_def(level_name, guid)

        def _yaml_merge(root, parent):
            for k, v in parent.items():
                if k not in root:
                    root[k] = v
                elif isinstance(v, dict):
                    _yaml_merge(root[k], v)

        def _preprocess(ent_def, root):
            if 'prefab' in ent_def:
                prefab_f = self.rpc.call_service("filesystem_service", "read", path="src/%s.entity" % ent_def['prefab'])
                prefab_def = yaml.load(prefab_f)

                _preprocess(prefab_def, root)
                _yaml_merge(root, prefab_def)

        final = {}
        ent_def = level_def['entities'][guid]
        _preprocess(ent_def, final)

        return final

    def _subcribe_level_inspector(self, msg):
        type = msg['type']

        if type == 'entity_selected':
            guid = msg['guid']
            level_name = msg['level_name']

            self.property_tree.clear()
            entity_def = self._load_entity_def(level_name, guid)

            for component in entity_def['components'].values():
                component_type = component['component_type']

                if self.tc.has[component_type](guid):
                    component_item = QTreeWidgetItem([self.tc.name[component_type], ])
                    self.property_tree.addTopLevelItem(component_item)

                    for prop_name in self.tc.properties_ordered[component_type]:
                        if prop_name not in component:
                            continue

                        value = component[prop_name]

                        prop_body = self.tc.properties[component_type][prop_name]

                        name = prop_body['text']

                        item = QTreeWidgetItem([name])
                        component_item.addChild(item)

                        _type = prop_body["type"]

                        if _type == 'vec3f':
                            self._create_vec3f(item, guid, component_type, prop_name, prop_body, value)

                        elif _type == 'string':
                            self._create_string(item, guid, component_type, prop_name, prop_body, value)

                        elif _type == 'asset':
                            self._create_asset(item, guid, component_type, prop_name, prop_body, value)

            self.property_tree.expandAll()

    def _parse_data(self, data):
        for group_name, group_entities in data.items():
            parent = self._add_groups(self.model, group_name)

            for entity_guid, entity_body in group_entities.items():
                ent_name = entity_body['name']

                self._add_entities(self.model,
                                   "%s (%s)" % (ent_name, entity_guid), parent)

        self.entities_tree.expandAll()
