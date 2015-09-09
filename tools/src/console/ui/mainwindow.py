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
        MainWindow.resize(850, 479)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.horizontalLayout = QtWidgets.QHBoxLayout(self.centralwidget)
        self.horizontalLayout.setObjectName("horizontalLayout")
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 850, 21))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)
        self.log_dock = QtWidgets.QDockWidget(MainWindow)
        self.log_dock.setObjectName("log_dock")
        self.dockWidgetContents = QtWidgets.QWidget()
        self.dockWidgetContents.setObjectName("dockWidgetContents")
        self.verticalLayout = QtWidgets.QVBoxLayout(self.dockWidgetContents)
        self.verticalLayout.setObjectName("verticalLayout")
        self.LogWidget = QtWidgets.QTreeWidget(self.dockWidgetContents)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.LogWidget.sizePolicy().hasHeightForWidth())
        self.LogWidget.setSizePolicy(sizePolicy)
        self.LogWidget.setRootIsDecorated(False)
        self.LogWidget.setObjectName("LogWidget")
        self.LogWidget.header().setDefaultSectionSize(100)
        self.LogWidget.header().setSortIndicatorShown(False)
        self.verticalLayout.addWidget(self.LogWidget)
        self.log_dock.setWidget(self.dockWidgetContents)
        MainWindow.addDockWidget(QtCore.Qt.DockWidgetArea(1), self.log_dock)
        self.dockWidget_2 = QtWidgets.QDockWidget(MainWindow)
        self.dockWidget_2.setObjectName("dockWidget_2")
        self.dockWidgetContents_3 = QtWidgets.QWidget()
        self.dockWidgetContents_3.setObjectName("dockWidgetContents_3")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.dockWidgetContents_3)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.command_log_textedit = QtWidgets.QTextEdit(self.dockWidgetContents_3)
        self.command_log_textedit.setObjectName("command_log_textedit")
        self.verticalLayout_2.addWidget(self.command_log_textedit)
        self.command_lineedit = QtWidgets.QLineEdit(self.dockWidgetContents_3)
        self.command_lineedit.setObjectName("command_lineedit")
        self.verticalLayout_2.addWidget(self.command_lineedit)
        self.dockWidget_2.setWidget(self.dockWidgetContents_3)
        MainWindow.addDockWidget(QtCore.Qt.DockWidgetArea(4), self.dockWidget_2)

        self.retranslateUi(MainWindow)
        self.command_lineedit.returnPressed.connect(MainWindow.send_command)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "cetech console"))
        self.log_dock.setWindowTitle(_translate("MainWindow", "Log"))
        self.LogWidget.headerItem().setText(0, _translate("MainWindow", "Level"))
        self.LogWidget.headerItem().setText(1, _translate("MainWindow", "Where"))
        self.LogWidget.headerItem().setText(2, _translate("MainWindow", "Message"))
        self.dockWidget_2.setWindowTitle(_translate("MainWindow", "&Console"))

