from PyQt5.QtCore import Qt


class ScriptFrontendService(object):
    def __init__(self, frontend_api):
        """
        :type frontend_api: cetech_playground.frontend.FrontendApp
        """

        self._frontend = frontend_api

        self._frontend.subscribe_service("asset_service", self._subcribe_asset_browser)

        self.filename_editor_map = dict()

    def close(self):
        for window_name in self.filename_editor_map.values():
            w = self._frontend.get_window(window_name)
            w.close()

    def _window_name(self, asset_type, asset_name):
        return "script_editor_%s_%s" % (asset_type, asset_name)

    def _subcribe_asset_browser(self, msg):
        msg_type = msg['msg_type']

        asset_name = msg['name']
        asset_type = msg['type']
        asset_path = msg['path']

        if msg_type == 'dclick' and asset_type != 'level':
            if asset_path in self.filename_editor_map:
                w = self._frontend.get_dock(self.filename_editor_map[asset_path])
                w.raise_()
                return

            window_name = self._window_name(asset_type, asset_name)
            title = "Script editor: %s.%s" % (asset_name, asset_type)

            self._frontend.create_window(window_name, title, menu="script_editor_menu")

            self.filename_editor_map[asset_path] = window_name

            self._frontend.create_dock(window_name, title, window_name, parent="main_window", parent_area='top')
            dock = self._frontend.get_dock(window_name)
            dock.setProperty("filename", asset_path)
            dock.setAttribute(Qt.WA_DeleteOnClose)
            dock.destroyed.connect(self._editor_destroyed)

            if len(self.filename_editor_map) > 1:
                root_w = self._frontend.get_window("main_window")
                d = self._frontend.get_dock(tuple(self.filename_editor_map.values())[0])
                root_w.tabifyDockWidget(d, dock)

            self._frontend.create_web_content(window_name, window_name,
                                              "script_editor/static/script_editor.html"
                                              "?src/%s.%s" % (
                                                  asset_name, asset_type)
                                              )

    def _editor_destroyed(self, obj):
        del self.filename_editor_map[obj.property("filename")]
