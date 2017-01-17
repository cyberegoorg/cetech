import os
import platform

from PyQt5 import QtWebKitWidgets
from PyQt5 import QtWidgets
from PyQt5.QtCore import QUrl
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QCursor
from PyQt5.QtWebKit import QWebSettings
from PyQt5.QtWidgets import QAction, QWidget
from PyQt5.QtWidgets import QDockWidget
from PyQt5.QtWidgets import QMainWindow, QTabWidget
from PyQt5.QtWidgets import QMenu
from PyQt5.QtWidgets import QMenuBar


class Action(object):
    def __init__(self, text, type, fronted, window_name, script=None, shortcut=None, **kwargs):
        self.text = text
        self.type = type
        self.script = script
        self.shortcut = shortcut
        self.fronted = fronted
        self.window_name = window_name

        window = fronted.get_window(window_name)

        action = QAction(window)
        action.setText(text)

        if shortcut is not None:
            action.setShortcut(shortcut)
            action.setShortcutContext(Qt.WidgetWithChildrenShortcut)

        if type == "python":
            c = compile(script.strip(), "exec", "exec")
            l = {}
            exec(c, {}, l)
            on_triggered = l['on_triggered']
            action.triggered.connect(lambda: on_triggered(self.fronted, self.window_name))

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


QT_AREA_MAP = dict(
    bottom=Qt.BottomDockWidgetArea,
    top=Qt.TopDockWidgetArea,
    left=Qt.LeftDockWidgetArea,
    right=Qt.RightDockWidgetArea
)


class QtFrontendGui(object):
    def __init__(self, frontend):
        self.frontend = frontend

        self.actions = {}
        self.menus = {}
        self.menus_template = {}
        self.action_template = {}
        self.windows = {}
        self.docks = {}
        self.contents = {}
        self.frontend_service = {}

    def create_window(self, name, title, parent=None, parent_area='bottom', show=True, menu=None):
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

            self.windows[parent].addDockWidget(QT_AREA_MAP[parent_area], dock)
            self.docks[name] = dock

        if menu is not None:
            self._create_menu(self.menus_template[menu], name)

    def create_dock(self, name, title, window, parent, parent_area='bottom', show=True):
        dock = QDockWidget(self.windows[parent])
        dock.setWindowTitle(title)
        dock.setWidget(self.windows[window])

        self.windows[window].setParent(dock)

        if show:
            dock.show()

        self.windows[parent].addDockWidget(QT_AREA_MAP[parent_area], dock)
        self.docks[name] = dock

    def create_web_content(self, name, window_name, url):
        full_url = "http://localhost:8080/modules/%s/" % url

        w = self.windows[window_name]

        web_view = QtWebKitWidgets.QWebView(w)
        web_view.page().settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)
        web_view.page().mainFrame().setUrl(QUrl(full_url))

        w.setCentralWidget(web_view)

        self.contents[name] = web_view

    def _create_menu(self, menus, window_name):
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
                menu = self.menus["%s:%s" % (window_name, path)]
                action_name = m['action']
                template = self.action_template[action_name]

                action = Action(fronted=self.frontend, window_name=window_name, **template)
                self.actions["%s:%s" % (window_name, name)] = action

                menu.addAction(action.action)

            else:
                window = self.windows[window_name]

                menu = QMenu(window)
                menu.setTitle(name)
                window.menubar.addAction(menu.menuAction())

                self.menus["%s:%s" % (window_name, m['path'])] = menu

    def _load_menus(self, menus):
        for m in menus:
            name = m['name']
            items = m['items']

            if name in self.menus_template:
                self.menus_template[name].extend(items)
            else:
                self.menus_template[name] = items

    def _load_actions(self, actions):
        for a in actions:
            name = a['name']
            self.action_template[name] = a

    def _load_window(self, window_def, windows):
        name = window_def['name']
        title = window_def['title']
        show = window_def.get('show', False)
        parent = window_def.get('parent', None)
        parent_area = window_def.get('parent_area', 'bottom')
        menu = window_def.get('menu', None)

        if parent is not None and parent not in self.windows:
            self._load_window(windows[parent], windows)

        self.create_window(name, title, parent=parent, show=show, parent_area=parent_area, menu=menu)

    def _load_windows(self, windows):
        w = dict()

        for a in windows:
            w[a['name']] = a

        for w_def in w.values():
            if w_def['name'] not in self.windows:
                self._load_window(w_def, w)

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

                content_instance = content_global[script_class](frontend=self.frontend)

                w.setCentralWidget(content_instance)

                self.contents[name] = content_instance

    def _load_frontend_service(self, contents):
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

            content_instance = content_global[script_class](frontend_api=self.frontend)

            self.frontend_service[name] = content_instance

    def register_to_module_manager(self, module_manager):
        module_manager.register_module_type("actions", self._load_actions)
        module_manager.register_module_type("menus", self._load_menus)
        module_manager.register_module_type("windows", self._load_windows)
        module_manager.register_module_type("window_contents", self._load_window_contents)
        module_manager.register_module_type("frontend_service", self._load_frontend_service)

    def close(self):
        for c in self.frontend_service.values():
            c.close()

        for w in self.windows.values():
            w.close()


class CETechWiget(QWidget):
    def __init__(self, rpc, instance_name, parent=None):
        self.rpc = rpc
        self.instance_name = instance_name

        self.last_pos = (0, 0)
        self.instance = None

        self.start_pos = None
        self.move_skip = False

        self.left = False
        self.midle = False
        self.right = False

        super(CETechWiget, self).__init__(flags=Qt.ForeignWindow, parent=parent)

        self.setFocusPolicy(Qt.ClickFocus)

    def set_instance(self, instance):
        self.instance = instance

    @property
    def ready(self):
        return True
        # return self.instance.ready if self.instance is not None else False

    @property
    def wid(self):
        # TODO
        if platform.system().lower() == 'darwin':
            return None
        else:
            return int(self.winId())

    def resizeEvent(self, event):
        if not self.ready:
            return

        size = event.size()

        self.rpc.call_service("engine_service", "call", instance_name=self.instance_name, fce_name="resize",
                              w=size.width(),
                              h=size.height())

        event.accept()

    def _move_mouse(self, x, y, left, midle, right):
        if not self.ready:
            return

        self.rpc.call_service("engine_service", "call", instance_name=self.instance_name, fce_name="lua_execute",
                              script="EditorInput:Move(%d, %d, %s, %s, %s)" % (x, y, left, midle, right))

    def mouseMoveEvent(self, event):
        """ QWidget.mouseMoveEvent(QMouseEvent)
        :type event: PyQt5.QtGui.QMouseEvent.QMouseEvent
        """

        if not self.ready:
            return

        if self.move_skip:
            self.move_skip = False
            return

        if not self.start_pos:
            self.start_pos = event.globalPos()

        if self.left or self.right:
            self.move_skip = True
            QCursor.setPos(self.start_pos)

        else:
            self.start_pos = None

        if self.start_pos is not None:
            self._move_mouse(self.start_pos.x() - event.globalPos().x(),
                             self.start_pos.y() - event.globalPos().y(),
                             str(self.left).lower(),
                             str(self.midle).lower(),
                             str(self.right).lower())

    def mousePressEvent(self, event):  # real signature unknown; restored from __doc__
        """ QWidget.mousePressEvent(QMouseEvent) """

        buttons = event.buttons()

        self.left = buttons == Qt.LeftButton
        self.midle = buttons == Qt.MidButton
        self.right = buttons == Qt.RightButton

        self.setCursor(Qt.BlankCursor)

    def mouseReleaseEvent(self, event):
        """ QWidget.mouseReleaseEvent(QMouseEvent) """
        buttons = event.buttons()

        self.setCursor(Qt.ArrowCursor)

    def keyPressEvent(self, event):
        """ QWidget.keyPressEvent(QKeyEvent)
        :type event: PyQt5.QtGui.QKeyEvent
        """
        if not self.ready:
            return

        btn = event.text()

        btn_map = {
            "w": "up",
            "s": "down",
            "a": "left",
            "d": "right",
        }

        if btn in btn_map:
            self.rpc.call_service("engine_service", "call", instance_name=self.instance_name, fce_name="lua_execute",
                                  script="EditorInput.keyboard.%s = true" % (btn_map[btn]))

    def keyReleaseEvent(self, event):
        """ QWidget.keyReleaseEvent(QKeyEvent)
        :type event: PyQt5.QtGui.QKeyEvent
        """
        if not self.ready:
            return

        btn = event.text()

        btn_map = {
            "w": "up",
            "s": "down",
            "a": "left",
            "d": "right",
        }

        if btn in btn_map:
            self.rpc.call_service("engine_service", "call", instance_name=self.instance_name, fce_name="lua_execute",
                                  script="EditorInput.keyboard.%s = false" % (btn_map[btn]))
