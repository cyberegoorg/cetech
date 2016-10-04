import platform

from cetech.consoleapi import ConsoleAPI
from playground.core.widget import CETechWiget


class AssetViewWidget(CETechWiget):
    def __init__(self, parent):
        self.project = None

        api = ConsoleAPI("ws://localhost:5576")
        api.connect()

        super(AssetViewWidget, self).__init__(parent, api=api, log_url=b"ws://localhost:5577")

    def open_asset(self, asset_name, type):
        self.api.lua_execute("AssetViewWidget:show_asset(\"%s\", \"%s\")" % (asset_name, type))

    def open_project(self, project):
        """
        :type project: cetech.project.Project
        """
        self.project = project

        # TODO
        if platform.system().lower() == 'darwin':
            wid = None
        else:
            wid = self.winId()

        self.set_instance(self.project.run_develop("AssetViewWidget", wid=wid, port=5576,
                                                   bootscript="playground/assetview_boot"))

    def close_project(self):
        self.api.quit()
        self.api.disconnect()

        # def resizeEvent(self, event):
        #     event.accept()
