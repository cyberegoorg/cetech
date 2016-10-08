import msgpack
from nanomsg import Socket, REQ, NanoMsgAPIError


class ConsoleProxy(object):
    def __init__(self, url):
        self.url = url.encode()
        self.socket = Socket(REQ)

        self.socket.recv_timeout = 100
        self.socket.send_timeout = 100

        super(ConsoleProxy, self).__init__()

    def connect(self):
        self.socket.connect(self.url)

    def disconnect(self):
        self.socket.close()

    # TODO: dangerous, block
    def send_command_norcv(self, cmd_name, echo=False, **kwargs):
        command = {
            "name": cmd_name,
            "args": kwargs
        }

        dump = msgpack.packb(command)

        if echo:
            print('send: ', command)

        try:
            self.socket.send(dump)
        except NanoMsgAPIError as e:
            print('send_error', e.msg)
            pass

    def send_command(self, cmd_name, echo=True, **kwargs):
        self.send_command_norcv(cmd_name=cmd_name, echo=echo, **kwargs)

        try:
            recv = self.socket.recv()
            unpack_msg = msgpack.unpackb(recv, encoding='utf-8')

            if echo:
                print('recv: ', recv)
                print(unpack_msg)

            return unpack_msg
        except NanoMsgAPIError as e:
            print('recv_error', e.msg)
            pass
