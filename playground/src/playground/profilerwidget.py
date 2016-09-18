from PyQt5.QtCore import QUrl, QDir
from PyQt5.QtWebKit import QWebSettings
from PyQt5.QtWidgets import QFrame

from playground.playground import Ui_ProfilerWidget


class ProfilerWidget(QFrame, Ui_ProfilerWidget):
    def __init__(self, api):
        super(ProfilerWidget, self).__init__()
        self.setupUi(self)

        self.api = api

        page = self.profile_webview.page()
        page.settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)

        self.profile_webview.page().mainFrame().setUrl(QUrl("file:///%s/html/profiler.html" % QDir.currentPath()))
