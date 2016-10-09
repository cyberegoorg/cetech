from collections import OrderedDict

from PyQt5.QtWidgets import QDockWidget


class ModuleInstance(object):
    def __init__(self, widget, dock):
        self.widget = widget
        self.dock = dock


class ModuleManager(object):
    def __init__(self, main_window):
        self.modules_instance = OrderedDict()
        self.main_window = main_window

    def init_modules(self, modules):
        for module in modules:
            module(module_manager=self)

    def new_common(self, module_instance, name):
        self.modules_instance[name] = ModuleInstance(module_instance, None)

    def new_docked(self, module_instance, name, title, dock_area, hiden=False):
        dock = QDockWidget()
        dock.setWindowTitle(title)
        dock.setWidget(module_instance)

        if hiden:
            dock.hide()
        else:
            dock.show()

        self.main_window.addDockWidget(dock_area, dock)
        self.modules_instance[name] = ModuleInstance(module_instance, dock)

    def __getitem__(self, item):
        """Get module instance by name

        :rtype: playground.core.modules.ModuleInstance
        """
        return self.modules_instance[item]

    def open_project(self, project):
        for v in self.modules_instance.values():
            v.widget.open_project(project)
