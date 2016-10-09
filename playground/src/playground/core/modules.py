from collections import OrderedDict

from PyQt5.QtWidgets import QDockWidget


class PlaygroundModule(object):
    def __init__(self):
        self.modules_manager = None

    def init_module(self, module_manager):
        self.modules_manager = module_manager

    def show_asset(self, path, name, ext):
        pass

    def close_project(self):
        pass

    def open_asset(self, path, name, ext):
        return False

    def open_project(self, project):
        pass


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

    def show_asset(self, path, name, ext):
        for k, v in self.modules_instance.items():
            if v.widget.show_asset(path, name, ext):
                break

    def open_asset(self, path, name, ext):
        for k, v in self.modules_instance.items():
            if v.widget.open_asset(path, name, ext):
                break

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

    def close_project(self):
        for v in self.modules_instance.values():
            v.widget.close_project()
