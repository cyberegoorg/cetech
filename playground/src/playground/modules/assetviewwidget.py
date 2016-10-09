import platform

from PyQt5.QtCore import Qt

from cetech.consoleapi import ConsoleAPI
from playground.core.modules import PlaygroundModule
from playground.core.widget import CETechWiget


class AssetViewWidget(CETechWiget, PlaygroundModule):
    def __init__(self, module_manager):
        super(AssetViewWidget, self).__init__()

        self.init_module(module_manager)

        self.project = None

        self.modules_manager.new_docked(self, "asset_view", "Asset preview",
                                        Qt.BottomDockWidgetArea)

    def show_asset(self, path, name, type):
        self.instance.console_api.lua_execute("AssetView:show_asset(\"%s\", \"%s\")" % (name, type))

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
