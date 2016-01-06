import msgpack
from nanomsg import Socket, REQ, PUSH


class ConsoleProxy(object):
    def __init__(self, url):
        super(ConsoleProxy, self).__init__()

        self.socket = Socket(PUSH)
        self.socket.connect(url)

    def send_command(self, cmd_name, **kwargs):
        command = {
            "name": cmd_name,
            "args": kwargs
        }

        dump = msgpack.packb(command)
        self.socket.send(dump)

        # recv = self.socket.recv()
        # msgpack.unpackb(recv)

    def disconnect(self):
        self.socket.close()
