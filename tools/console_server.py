import enet
import json
import threading


class ConsoleProxy(threading.Thread):
    def __init__(self, address, port):
        super(ConsoleProxy, self).__init__()

        host = enet.Host(None, 1, 1, 0, 0)
        self.address = enet.Address(address, port)
        self.host = host
        self.peer = self.host.connect(self.address, 0)

        self.connect = False
        self.disconnecting = False

        self.on_log = None
        self.on_perf_timers = None
        self.on_lua_return = None

    def disconnect(self):
        self.disconnecting = True

    def run(self):
        while not self.disconnecting:
            self._tick()

        if self.connect:
            self.peer.disconnect(0)
            while True:
                event = self.host.service(0)
                if event.type == enet.EVENT_TYPE_DISCONNECT:
                    break

    def tick(self):
        if not self.disconnecting:
            self._tick()
        else:
            self.peer.disconnect(0)
            self.host.service(0)

        return True

    def _tick(self):
        event = self.host.service(0)
        if event.type == enet.EVENT_TYPE_CONNECT:
            self.connect = True

        elif event.type == enet.EVENT_TYPE_DISCONNECT:
            self.connect = False

        elif event.type == enet.EVENT_TYPE_RECEIVE:
            data = json.loads(event.packet.data.decode("utf-8"))

            if data["type"] == 'log':
                if self.on_log:
                    self.on_log(data["level"], data["where"], data['msg'])

            elif data["type"] == 'perf_timers':
                if self.on_perf_timers:
                    self.on_perf_timers(data["timers"])

                    # elif data["type"] == 'lua_return':
                    #     if self.on_lua_return:
                    #         print(data)
                    #         self.on_lua_return(data['return'])

    def send_command(self, cmd_name, **kwargs):
        if not self.connect:
            return

        command_json = {
            "name": cmd_name,
            "args": kwargs
        }

        dump = str.encode('%s\0' % json.dumps(command_json))
        packet = enet.Packet(dump, enet.PACKET_FLAG_RELIABLE)

        self.peer.send(0, packet)
        self.host.flush()


cp = ConsoleProxy("localhost", 2222)
cp.start()
cp.send_command("lua.execute", script="print(\"fofofofofofofo\")")
