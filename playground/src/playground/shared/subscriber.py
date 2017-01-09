import threading
from collections import defaultdict
from time import sleep

from PyQt5 import QtCore

from cetech.consoleproxy import NanoSub


class QTSubscriber(QtCore.QThread):
    _on_msg = QtCore.pyqtSignal(object)

    def __init__(self, url):
        super().__init__()

        self._run = False
        self._sub = NanoSub(url, echo=False)
        self._service_subscribers = defaultdict(list)
        self._on_msg.connect(self._subcriber)

    def subscribe_service(self, service_name, subscribe_clb):
        self._service_subscribers[service_name].append(subscribe_clb)

    def run(self):
        self._run = True

        self._sub.connect()

        while self._run:
            msg = self._sub.try_recive()

            if msg is None:
                sleep(0)
                continue

            self._on_msg.emit(msg)

        self._sub.close()

    def close(self):
        self._run = False

    def _subcriber(self, msg):
        if msg['msg_type'] == 'event':
            service = msg['service']

            for clb in self._service_subscribers[service]:
                clb(msg['msg'])


class PythonSubscriber(threading.Thread):
    def __init__(self, url):
        super().__init__()

        self._run = False
        self._sub = NanoSub(url, echo=False)
        self._service_subscribers = defaultdict(list)

    def subscribe_service(self, service_name, subscribe_clb):
        self._service_subscribers[service_name].append(subscribe_clb)

    def run(self):
        self._run = True

        self._sub.connect()

        while self._run:
            msg = self._sub.try_recive()

            if msg is None:
                sleep(0)
                continue

            if msg['msg_type'] == 'event':
                service = msg['service']

                for clb in self._service_subscribers[service]:
                    clb(msg['msg'])

        self._sub.close()

    def close(self):
        self._run = False
