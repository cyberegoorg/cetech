# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'src/console/ui/consolewidget.ui'
#
# Created by: PyQt5 UI code generator 5.5
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_ConsoleWidget(object):
    def setupUi(self, ConsoleWidget):
        ConsoleWidget.setObjectName("ConsoleWidget")
        ConsoleWidget.resize(398, 298)
        ConsoleWidget.setFrameShape(QtWidgets.QFrame.StyledPanel)
        ConsoleWidget.setFrameShadow(QtWidgets.QFrame.Raised)
        self.verticalLayout = QtWidgets.QVBoxLayout(ConsoleWidget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.command_log_textedit = QtWidgets.QTextEdit(ConsoleWidget)
        self.command_log_textedit.setObjectName("command_log_textedit")
        self.verticalLayout.addWidget(self.command_log_textedit)
        self.command_lineedit = QtWidgets.QLineEdit(ConsoleWidget)
        self.command_lineedit.setObjectName("command_lineedit")
        self.verticalLayout.addWidget(self.command_lineedit)

        self.retranslateUi(ConsoleWidget)
        self.command_lineedit.returnPressed.connect(ConsoleWidget.send_command)
        QtCore.QMetaObject.connectSlotsByName(ConsoleWidget)

    def retranslateUi(self, ConsoleWidget):
        _translate = QtCore.QCoreApplication.translate
        ConsoleWidget.setWindowTitle(_translate("ConsoleWidget", "Frame"))

