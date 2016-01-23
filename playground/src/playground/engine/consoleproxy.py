import msgpack
from nanomsg import Socket, REQ


class ConsoleProxy(object):
    def __init__(self, url):
        super(ConsoleProxy, self).__init__()

        self.socket = Socket(REQ)
        self.socket.connect(url)

    def send_command(self, cmd_name, **kwargs):
        command = {
            "name": cmd_name,
            "args": kwargs
        }

        dump = msgpack.packb(command)
        print('send: ', command)
        self.socket.send(dump)

        print('recv: ')
        recv = self.socket.recv()
        print(msgpack.unpackb(recv, encoding='utf-8'))

    def disconnect(self):
        self.socket.close()
