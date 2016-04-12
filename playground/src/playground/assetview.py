import os
import platform

from playground.engine.cetechwidget import CetechWidget, ReadyLock
from playground.engine.consoleapi import ConsoleAPI


class AssetView(CetechWidget):
    def __init__(self, parent):
        self.project = None

        api = ConsoleAPI(b"tcp://localhost:5577")
        api.connect()

        super(AssetView, self).__init__(parent, api=api, log_url=b"ws://localhost:5576")

    def open_asset(self, asset_name, type):
        self.api.lua_execute("AssetView:show_asset(\"%s\", \"%s\")" % (asset_name, type))

    def open_project(self, project):
        """
        :type project: playground.engine.cetechproject.CetechProject
        """
        #                     /\     /\      /\             /\       WTF???? REFACTOR THIS !!!
        self.project = project

        if platform.system().lower() == 'darwin':
            wid = None
        else:
            wid = self.winId()

        self.project.run_cetech_develop("AssetView", wid=wid, port=5576,
                                        bootscript=os.path.join("playground", "assetview_boot"))

    def close_project(self):
        self.api.quit()
        self.api.disconnect()

    # def resizeEvent(self, event):
    #     event.accept()
