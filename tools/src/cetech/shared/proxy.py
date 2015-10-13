import enet
import json
from time import sleep


class ConsoleProxy(object):
    def __init__(self, address, port, service_sleep):
        super(ConsoleProxy, self).__init__()

        self.service_sleep = service_sleep

        host = enet.Host(None, 1, 10, 0, 0)
        self.address = enet.Address(address, port)
        self.host = host
        self.peer = self.host.connect(self.address, 10)

        self.connected = False
        self.disconnecting = False

        self.handlers = {}

        self.on_log = []

    def register_handler(self, type, handler):
        if type not in self.handlers:
            self.handlers[type] = []

        self.handlers[type].append(handler)

    def disconnect(self):
        self.disconnecting = True

    def run_loop(self):
        while not self.disconnecting:
            self._tick()

        self._safe_disconnect()

    def _safe_disconnect(self):
        if self.connected:
            self.peer.disconnect_now()
            self.connected = False
            self.disconnecting = False

    def tick(self):
        if not self.disconnecting:
            self._tick()
        else:
            self._safe_disconnect()

        return True

    def _parse_message(self, message):
        self._call_handler(**message)

    def _tick(self):
        event = self.host.service(0)
        if event.type == enet.EVENT_TYPE_NONE: self.service_sleep()

        if event.type == enet.EVENT_TYPE_CONNECT:
            self.connected = True

        elif event.type == enet.EVENT_TYPE_DISCONNECT:
            self.connected = False

        elif event.type == enet.EVENT_TYPE_RECEIVE:
            s = event.packet.data.decode("utf-8")
            message = json.loads(s)
            self._parse_message(message)

    def _call_handler(self, type, **kwargs):
        if type not in self.handlers:
            return

        handlers = self.handlers[type]
        for h in handlers:
            h(**kwargs)

    def send_command(self, cmd_name, **kwargs):
        if not self.connected:
            return

        command_json = {
            "name": cmd_name,
            "args": kwargs
        }

        dump = str.encode('%s\0' % json.dumps(command_json))
        packet = enet.Packet(dump, enet.PACKET_FLAG_RELIABLE)

        self.peer.send(0, packet)
        self.host.flush()
