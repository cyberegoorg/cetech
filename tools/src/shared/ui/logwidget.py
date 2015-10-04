# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'src/shared/ui/logwidget.ui'
#
# Created by: PyQt5 UI code generator 5.5
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_LogWidget(object):
    def setupUi(self, LogWidget):
        LogWidget.setObjectName("LogWidget")
        LogWidget.resize(1103, 276)
        LogWidget.setFrameShape(QtWidgets.QFrame.StyledPanel)
        LogWidget.setFrameShadow(QtWidgets.QFrame.Raised)
        self.verticalLayout = QtWidgets.QVBoxLayout(LogWidget)
        self.verticalLayout.setObjectName("verticalLayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.log_tree_widget = QtWidgets.QTreeWidget(LogWidget)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Preferred, QtWidgets.QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.log_tree_widget.sizePolicy().hasHeightForWidth())
        self.log_tree_widget.setSizePolicy(sizePolicy)
        self.log_tree_widget.setRootIsDecorated(False)
        self.log_tree_widget.setObjectName("log_tree_widget")
        self.log_tree_widget.header().setDefaultSectionSize(100)
        self.verticalLayout.addWidget(self.log_tree_widget)

        self.retranslateUi(LogWidget)
        QtCore.QMetaObject.connectSlotsByName(LogWidget)

    def retranslateUi(self, LogWidget):
        _translate = QtCore.QCoreApplication.translate
        LogWidget.setWindowTitle(_translate("LogWidget", "Frame"))
        self.log_tree_widget.headerItem().setText(0, _translate("LogWidget", "Time"))
        self.log_tree_widget.headerItem().setText(1, _translate("LogWidget", "Level"))
        self.log_tree_widget.headerItem().setText(2, _translate("LogWidget", "Where"))
        self.log_tree_widget.headerItem().setText(3, _translate("LogWidget", "Message"))

