from PyQt5.QtCore import QUrl, QDir
from PyQt5.QtCore import Qt
from PyQt5.QtWebKit import QWebSettings
from PyQt5.QtWidgets import QFrame

from playground.core.modules import PlaygroundModule
from playground.ui.profilerwidget import Ui_ProfilerWidget


class ProfilerWidget(QFrame, Ui_ProfilerWidget, PlaygroundModule):
    def __init__(self, module_manager):
        super(ProfilerWidget, self).__init__()
        self.setupUi(self)
        self.init_module(module_manager)

        page = self.profile_webview.page()
        page.settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)

        self.profile_webview.page().mainFrame().setUrl(QUrl("file:///%s/html/profiler.html" % QDir.currentPath()))

        self.modules_manager.new_docked(self, "profiler", "Profiler",
                                        Qt.RightDockWidgetArea, hiden=True)
