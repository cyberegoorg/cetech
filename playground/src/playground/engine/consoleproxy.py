import msgpack
from nanomsg import Socket, REQ


class ConsoleProxy(object):
    def __init__(self, url):
        super(ConsoleProxy, self).__init__()

        self.socket = Socket(REQ)
        self.socket.connect(url)

    def send_command(self, cmd_name, **kwargs):
        command_json = {
            "name": cmd_name,
            "args": kwargs
        }

        dump = msgpack.packb(command_json)
        print(dump)
        self.socket.send(dump)

    def disconnect(self):
        self.socket.close()
