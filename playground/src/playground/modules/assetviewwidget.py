import platform

from cetech.consoleapi import ConsoleAPI
from playground.core.widget import CETechWiget


class AssetViewWidget(CETechWiget):
    def __init__(self, parent):
        self.project = None

        # api = ConsoleAPI("ws://localhost:5576")
        # api.connect()

        super(AssetViewWidget, self).__init__(parent)

    def open_asset(self, asset_name, type):
        self.instance.console_api.lua_execute("AssetView:show_asset(\"%s\", \"%s\")" % (asset_name, type))

    def open_project(self, project):
        """
        :type project: cetech.project.ProjectManager
        """
        self.project = project
        self.set_instance(self.project.run_develop("AssetViewWidget", wid=self.wid, port=5576,
                                                   bootscript="playground/assetview_boot"))

    def close_project(self):
        if self.ready:
            self.instance.console_api.quit()
            self.instance.console_api.disconnect()
