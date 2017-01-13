import playground.shared.modules as modules
from playground.frontend.gui_qt import QtFrontendGui
from playground.shared import rpc
from playground.shared.subscriber import QTSubscriber


class FrontendApp(object):
    def __init__(self, modules_dir, name, dir, core_dir):
        self.rpc = rpc.Client(url="ws://localhost:8888", recv_timeout=10 * 1000)
        self.rpc.connect()

        self.rpc.call_service("project_service", "open_project", name=name, project_dir=dir, core_dir=core_dir)

        self._modules_dir = modules_dir

        self._subscriber = QTSubscriber(url="ws://localhost:8889")
        self._subscriber.start()

        self._modules_manager = modules.Manager()

        self._gui = QtFrontendGui(self)
        self._gui.register_to_module_manager(self._modules_manager)

        self._modules_manager.load_in_path(modules_dir)

    def quit(self):
        self.close()

    def close(self):
        self._gui.close()
        self._subscriber.close()

    def create_window(self, name, title, parent=None, parent_area='bottom', show=True, menu=None):
        return self._gui.create_window(name=name, title=title, parent=parent, parent_area=parent_area,
                                       show=show, menu=menu)

    def create_dock(self, name, title, window, parent=None, parent_area='bottom', show=True):
        return self._gui.create_dock(name=name, title=title, window=window, parent=parent,
                                     parent_area=parent_area,
                                     show=show)

    def create_web_content(self, name, window_name, url):
        return self._gui.create_web_content(name=name, window_name=window_name, url=url)

    def get_window(self, name):
        return self._gui.windows[name]

    def get_dock(self, name):
        return self._gui.docks[name]

    def get_content(self, name):
        return self._gui.contents[name]

    def subscribe_service(self, service_name, subscribe_clb):
        return self._subscriber.subscribe_service(service_name=service_name, subscribe_clb=subscribe_clb)
