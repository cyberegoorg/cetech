import socket
import json


class ConsoleServerAPI(object):
    def __init__(self):
        self._socket = None
        self._msg_id = 1;

    def connect(self, host, port):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.connect((host, port))

    def disconnect(self):
        self._socket = None
        self._socket.close()

    def send_cmd(self, name, **kwargs):
        msg = {
            "name": name,
            "args": kwargs,
            "id": self._msg_id
        }

        self._socket.send(str.encode(json.dumps(msg)))
        self._msg_id += 1

    def tick(self):
        pass

    def lua_execute(self, script):
        self.send_cmd("lua.execute", script=script)

api = ConsoleServerAPI()
api.connect('localhost', 2222)
api.lua_execute("print('command from console server');")
