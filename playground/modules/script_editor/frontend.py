from PyQt5.QtCore import Qt


class ScriptManager(object):
    def __init__(self, frontend):
        """
        :type frontend: playground.frontend.app.FrontendApp
        """
        self.frontend = frontend

        self.frontend.subscribe_service("asset_service", self._subcribe_asset_browser)

        self.filename_editor_map = dict()

    def _window_name(self, asset_type, asset_name):
        return "script_editor_%s_%s" % (asset_type, asset_name)

    def _subcribe_asset_browser(self, msg):
        msg_type = msg['msg_type']

        asset_name = msg['name']
        asset_type = msg['type']
        asset_path = msg['path']

        if msg_type == 'dclick' and asset_type != 'level':
            if asset_path in self.filename_editor_map:
                w = self.frontend.get_dock(self.filename_editor_map[asset_path])
                w.raise_()
                return

            window_name = self._window_name(asset_type, asset_name)
            title = "Script editor: %s.%s" % (asset_name, asset_type)

            self.frontend.create_window(window_name, title)

            self.filename_editor_map[asset_path] = window_name

            self.frontend.create_dock(window_name, title, window_name, parent="main_window", parent_area='top')
            dock = self.frontend.get_dock(window_name)
            dock.setProperty("filename", asset_path)
            dock.setAttribute(Qt.WA_DeleteOnClose)
            dock.destroyed.connect(self._editor_destroyed)

            if len(self.filename_editor_map) > 1:
                root_w = self.frontend.get_window("main_window")
                d = self.frontend.get_dock(tuple(self.filename_editor_map.values())[0])
                root_w.tabifyDockWidget(d, dock)

            self.frontend.create_web_content(window_name, window_name,
                                             "http://localhost:8080/static/script_editor.html?%s.%s" % (
                                                 asset_name, asset_type))

    def _editor_destroyed(self, obj):
        del self.filename_editor_map[obj.property("filename")]

    def close(self):
        pass
