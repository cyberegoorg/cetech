import platform

from cetech.consoleapi import ConsoleAPI
from playground.core.widget import CETechWiget


class LevelWidget(CETechWiget):
    def __init__(self, parent):
        self.project = None

        super(LevelWidget, self).__init__(parent)

    def open_project(self, project):
        """
        :type project: cetech.project.ProjectManager
        """
        self.project = project

        self.set_instance(
            self.project.run_develop("LevelView", compile_=True, continue_=True, wid=self.wid,
                                     bootscript="playground/leveleditor_boot"))

    def open_level(self, level_name):
        if not self.ready:
            return

        self.instance.console_api.lua_execute("Editor:load_level(\"%s\")" % level_name)

    def close_project(self):
        if self.ready:
            self.instance.console_api.quit()
            self.instance.console_api.disconnect()
