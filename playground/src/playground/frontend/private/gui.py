import os

from PyQt5 import QtWebKitWidgets
from PyQt5 import QtWidgets
from PyQt5.QtCore import QUrl
from PyQt5.QtCore import Qt
from PyQt5.QtWebKit import QWebSettings
from PyQt5.QtWidgets import QAction
from PyQt5.QtWidgets import QDockWidget
from PyQt5.QtWidgets import QMainWindow, QTabWidget
from PyQt5.QtWidgets import QMenu
from PyQt5.QtWidgets import QMenuBar


class Action(object):
    def __init__(self, text, type, fronted_api, script=None, shortcut=None, **kwargs):
        self.text = text
        self.type = type
        self.script = script
        self.shortcut = shortcut
        self.fronted_api = fronted_api

        action = QAction()
        action.setText(text)

        if shortcut is not None:
            action.setShortcut('shortcut')

        if type == "python":
            c = compile(script.strip(), "exec", "exec")
            l = {}
            exec(c, {}, l)
            on_triggered = l['on_triggered']
            action.triggered.connect(lambda: on_triggered(self.fronted_api))

        self.action = action


class Window(QMainWindow):
    def __init__(self, name, title):
        super(Window, self).__init__()

        self.setWindowTitle(title)

        self.setTabPosition(Qt.AllDockWidgetAreas, QTabWidget.North)

        self.setDockOptions(
            QtWidgets.QMainWindow.AllowNestedDocks |
            QtWidgets.QMainWindow.AllowTabbedDocks |
            QtWidgets.QMainWindow.AnimatedDocks

        )

        self._create_main_menu()
        self.name = name

    def _create_main_menu(self):
        self.menubar = QMenuBar(self)
        self.setMenuBar(self.menubar)

    def closeEvent(self, evnt):
        if self.centralWidget() is not None:
            self.centralWidget().close()

        evnt.accept()


AREA_MAP = dict(
    bottom=Qt.BottomDockWidgetArea,
    top=Qt.TopDockWidgetArea,
    left=Qt.LeftDockWidgetArea,
    right=Qt.RightDockWidgetArea
)


class FrontendGui(object):
    def __init__(self, frontend):
        self.frontend = frontend

        self.actions = {}
        self.menus = {}
        self.windows = {}
        self.docks = {}
        self.contents = {}
        self.frontend_contents = {}

    def create_window(self, name, title, parent=None, parent_area='bottom', show=True):
        window = Window(name, title)

        if show and not parent:
            window.show()

        self.windows[name] = window

        if parent is not None:
            dock = QDockWidget(self.windows[parent])
            dock.setWindowTitle(title)
            dock.setWidget(window)

            if show:
                dock.show()

            self.windows[parent].addDockWidget(AREA_MAP[parent_area], dock)
            self.docks[name] = dock

    def create_dock(self, name, title, window, parent, parent_area='bottom', show=True):
        dock = QDockWidget(self.windows[parent])
        dock.setWindowTitle(title)
        dock.setWidget(self.windows[window])

        if show:
            dock.show()

        self.windows[parent].addDockWidget(AREA_MAP[parent_area], dock)
        self.docks[name] = dock

    def create_web_content(self, name, window_name, url):
        w = self.windows[window_name]

        web_view = QtWebKitWidgets.QWebView(w)
        web_view.page().settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)

        web_view.page().mainFrame().setUrl(QUrl(url))

        w.setCentralWidget(web_view)

        self.contents[name] = web_view

    def _load_menus(self, menus):
        def _sort(x):
            class K:
                def __init__(self, path, order):
                    self.path = path
                    self.order = order

                def __lt__(self, other):
                    return len(self.path) < len(other.path) and self.order < other.order

            path = x['path']
            order = x.get('order', 99999)

            return K(path, order)

        menus = sorted(menus, key=_sort)

        for m in menus:
            path = m['path'].split('/')
            name = m.get('text', "")
            path = '/'.join(path[:-1])

            if 'action' in m:
                menu = self.menus[path]
                menu.addAction(self.actions[m['action']].action)

            else:
                window_name = m.get("window", "main_window")
                window = self.windows[window_name]

                menu = QMenu(window)
                menu.setTitle(name)
                window.menubar.addAction(menu.menuAction())

                self.menus[m['path']] = menu

    def _load_actions(self, actions):
        for a in actions:
            name = a['name']
            action = Action(fronted_api=self.frontend.api, **a)
            self.actions[name] = action

    def _load_windows(self, windows):
        for a in windows:
            name = a['name']
            title = a['title']
            show = a.get('show', False)
            parent = a.get('parent', None)
            parent_area = a.get('parent_area', 'bottom')

            self.create_window(name, title, parent=parent, show=show, parent_area=parent_area)

    def _load_window_contents(self, contents):
        for a in contents:
            name = a['name']
            type = a['type']
            window = a['window']

            w = self.windows[window]

            if type == 'web_content':
                url = a['url']
                self.create_web_content(name=name, window_name=window, url=url)

            elif type == 'qt_content':
                filename = a['__filename__']
                script_file = a['script_file']
                script_class = a['script_class']

                script_file_path = os.path.join(os.path.dirname(filename), script_file)
                with open(script_file_path) as f:
                    compiled_service = compile(f.read(), script_file_path, "exec")

                content_global = {}
                exec(compiled_service, content_global)

                content_instance = content_global[script_class](frontend=self.frontend.api)

                w.setCentralWidget(content_instance)

                self.contents[name] = content_instance

    def _load_frontend_contents(self, contents):
        for a in contents:
            name = a['name']

            filename = a['__filename__']
            script_file = a['script_file']
            script_class = a['script_class']

            script_file_path = os.path.join(os.path.dirname(filename), script_file)
            with open(script_file_path) as f:
                compiled_content = compile(f.read(), script_file_path, "exec")

            content_global = {}
            exec(compiled_content, content_global)

            content_instance = content_global[script_class](frontend=self.frontend.api)

            self.frontend_contents[name] = content_instance

    def register_to_module_manager(self, module_manager):
        module_manager.register_module_type("actions", self._load_actions)
        module_manager.register_module_type("windows", self._load_windows)
        module_manager.register_module_type("menus", self._load_menus)
        module_manager.register_module_type("window_contents", self._load_window_contents)
        module_manager.register_module_type("frontend_contents", self._load_frontend_contents)

    def close(self):
        for c in self.frontend_contents.values():
            c.close()

        for w in self.windows.values():
            w.close()
