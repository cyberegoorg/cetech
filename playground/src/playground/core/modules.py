from collections import OrderedDict

from PyQt5.QtWidgets import QDockWidget


class PlaygroundModule(object):
    def __init__(self):
        self.modules_manager = None
        self.project_manager = None

    def init_module(self, module_manager):
        self.modules_manager = module_manager


class ModuleManager(object):
    def __init__(self, main_window):
        self.modules_instance = OrderedDict()
        self.modules_dock = OrderedDict()
        self.main_window = main_window

    def __getitem__(self, item):
        """Get module instance by name

        :rtype: playground.core.modules.PlaygroundModule
        """
        return self.modules_instance[item]

    def new_docked(self, widget, name, title, dock_area, hiden=False):
        dock = QDockWidget()
        dock.setWindowTitle(title)
        dock.setWidget(widget)

        if hiden:
            dock.hide()
        else:
            dock.show()

        self.main_window.addDockWidget(dock_area, dock)
        self.modules_dock[name] = dock

        return dock

    def init_modules(self, modules):
        for module_cls in modules:
            instance = module_cls(module_manager=self)

            self.modules_instance[module_cls.Name] = instance

    def show_asset(self, path, name, ext):
        for k, v in self.modules_instance.items():
            if hasattr(v, "on_show_asset"):
                if v.on_show_asset(path, name, ext):
                    break

    def open_asset(self, path, name, ext):
        for k, v in self.modules_instance.items():
            if hasattr(v, "on_open_asset"):
                if v.on_open_asset(path, name, ext):
                    break

    def open_project(self, project):
        for v in self.modules_instance.values():
            v.project_manager = project

            if hasattr(v, "on_open_project"):
                v.on_open_project(project)

    def close_project(self):
        for v in self.modules_instance.values():
            if hasattr(v, "on_close_project"):
                v.on_close_project()
