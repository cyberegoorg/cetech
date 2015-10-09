from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QFrame, QTreeWidgetItem

from cetech.shared.ui.recordeventswidget import Ui_RecordEventsWidget


class RecordEventWidget(QFrame, Ui_RecordEventsWidget):
    def __init__(self, api):
        super(RecordEventWidget, self).__init__()
        self.setupUi(self)

        self.api = api
        self.api.register_handler('debug_event', self.debug_event)

    def debug_event(self, etype, **kwargs):
        if etype not in ('EVENT_RECORD_FLOAT',):
            return


        name = kwargs['name']
        value = kwargs['value']

        items = self.record_treewidget.findItems(name, Qt.MatchExactly)

        if  not len(items):
            item = QTreeWidgetItem([name, str(value)])

            self.record_treewidget.addTopLevelItem(item)
        else:
            items[0].setText(1, str(value))

