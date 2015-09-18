# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'src/console/ui/mainwindow.ui'
#
# Created by: PyQt5 UI code generator 5.5
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(844, 473)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.centralwidget)
        self.horizontalLayout.setObjectName("horizontalLayout")
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 844, 21))
        self.menubar.setObjectName("menubar")
        self.menu_File = QtWidgets.QMenu(self.menubar)
        self.menu_File.setObjectName("menu_File")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)
        self.log_dock_widget = QtWidgets.QDockWidget(MainWindow)
        self.log_dock_widget.setFeatures(QtWidgets.QDockWidget.AllDockWidgetFeatures)
        self.log_dock_widget.setObjectName("log_dock_widget")
        self.dockWidgetContents = QtWidgets.QWidget()
        self.dockWidgetContents.setObjectName("dockWidgetContents")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.dockWidgetContents)
        self.verticalLayout.setObjectName("verticalLayout")
        self.log_dock_widget.setWidget(self.dockWidgetContents)
        MainWindow.addDockWidget(QtCore.Qt.DockWidgetArea(1), self.log_dock_widget)
        self.console_dock_widget = QtWidgets.QDockWidget(MainWindow)
        self.console_dock_widget.setFeatures(QtWidgets.QDockWidget.AllDockWidgetFeatures)
        self.console_dock_widget.setObjectName("console_dock_widget")
        self.dockWidgetContents_3 = QtWidgets.QWidget()
        self.dockWidgetContents_3.setObjectName("dockWidgetContents_3")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.dockWidgetContents_3)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.console_dock_widget.setWidget(self.dockWidgetContents_3)
        MainWindow.addDockWidget(QtCore.Qt.DockWidgetArea(4), self.console_dock_widget)
        self.repl_dock_widget = QtWidgets.QDockWidget(MainWindow)
        self.repl_dock_widget.setObjectName("repl_dock_widget")
        self.dockWidgetContents_2 = QtWidgets.QWidget()
        self.dockWidgetContents_2.setObjectName("dockWidgetContents_2")
        self.repl_dock_widget.setWidget(self.dockWidgetContents_2)
        MainWindow.addDockWidget(QtCore.Qt.DockWidgetArea(4), self.repl_dock_widget)
        self.recordevent_dock_widget = QtWidgets.QDockWidget(MainWindow)
        self.recordevent_dock_widget.setObjectName("recordevent_dock_widget")
        self.dockWidgetContents_5 = QtWidgets.QWidget()
        self.dockWidgetContents_5.setObjectName("dockWidgetContents_5")
        self.recordevent_dock_widget.setWidget(self.dockWidgetContents_5)
        MainWindow.addDockWidget(QtCore.Qt.DockWidgetArea(2), self.recordevent_dock_widget)
        self.action_Quit = QtWidgets.QAction(MainWindow)
        self.action_Quit.setObjectName("action_Quit")
        self.menu_File.addAction(self.action_Quit)
        self.menubar.addAction(self.menu_File.menuAction())

        self.retranslateUi(MainWindow)
        self.action_Quit.triggered.connect(MainWindow.close)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "cetech console"))
        self.menu_File.setTitle(_translate("MainWindow", "&File"))
        self.log_dock_widget.setWindowTitle(_translate("MainWindow", "Log"))
        self.console_dock_widget.setWindowTitle(_translate("MainWindow", "&Console"))
        self.repl_dock_widget.setWindowTitle(_translate("MainWindow", "&REPL"))
        self.recordevent_dock_widget.setWindowTitle(_translate("MainWindow", "Recor&d events"))
        self.action_Quit.setText(_translate("MainWindow", "&Quit"))
        self.action_Quit.setShortcut(_translate("MainWindow", "Ctrl+Q"))

