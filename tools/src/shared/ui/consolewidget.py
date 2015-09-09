# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'src/shared/ui/consolewidget.ui'
#
# Created by: PyQt5 UI code generator 5.5
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_ConsoleWidget(object):
    def setupUi(self, ConsoleWidget):
        ConsoleWidget.setObjectName("ConsoleWidget")
        ConsoleWidget.resize(396, 296)
        ConsoleWidget.setFrameShape(QtWidgets.QFrame.StyledPanel)
        ConsoleWidget.setFrameShadow(QtWidgets.QFrame.Raised)
        self.verticalLayout = QtWidgets.QVBoxLayout(ConsoleWidget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.command_log_textedit = QtWidgets.QTextEdit(ConsoleWidget)
        self.command_log_textedit.setReadOnly(True)
        self.command_log_textedit.setObjectName("command_log_textedit")
        self.verticalLayout.addWidget(self.command_log_textedit)
        self.command_lineedit = QtWidgets.QLineEdit(ConsoleWidget)
        self.command_lineedit.setToolTip("")
        self.command_lineedit.setObjectName("command_lineedit")
        self.verticalLayout.addWidget(self.command_lineedit)

        self.retranslateUi(ConsoleWidget)
        self.command_lineedit.returnPressed.connect(ConsoleWidget.send_command)
        QtCore.QMetaObject.connectSlotsByName(ConsoleWidget)

    def retranslateUi(self, ConsoleWidget):
        _translate = QtCore.QCoreApplication.translate
        ConsoleWidget.setWindowTitle(_translate("ConsoleWidget", "Frame"))
        self.command_log_textedit.setHtml(_translate("ConsoleWidget", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'Sans Serif\'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p></body></html>"))

