from PyQt5.QtCore import Qt

from playground.core.widget import CETechWiget


class LevelWidget(CETechWiget):
    def __init__(self, module_manager):
        self.project = None
        self.modules_manager = module_manager

        super(LevelWidget, self).__init__()

        self.modules_manager.new_docked(self, "level_editor", "Level editor", Qt.TopDockWidgetArea)

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
