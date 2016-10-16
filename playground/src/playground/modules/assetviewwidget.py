import platform

from PyQt5.QtCore import Qt

from cetech.consoleapi import ConsoleAPI
from playground.core.modules import PlaygroundModule
from playground.core.widget import CETechWiget


class AssetViewWidget(CETechWiget, PlaygroundModule):
    Name = "asset_view"

    def __init__(self, module_manager):
        super(AssetViewWidget, self).__init__()
        self.init_module(module_manager)

        self.dock = self.modules_manager.new_docked(self, self.Name, "Asset preview",
                                                    Qt.BottomDockWidgetArea)

    def on_show_asset(self, path, name, type):
        self.instance.console_api.lua_execute("AssetView:show_asset(\"%s\", \"%s\")" % (name, type))

    def on_open_project(self, project):
        """
        :type project: cetech.project.ProjectManager
        """
        self.set_instance(self.project_manager.run_develop("AssetViewWidget", wid=self.wid, port=5576,
                                                           bootscript="playground/assetview_boot"))

    def on_close_project(self):
        if self.ready:
            self.instance.console_api.quit()
            self.instance.console_api.disconnect()
