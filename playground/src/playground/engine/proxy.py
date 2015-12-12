import json

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

        dump = str.encode('%s\0' % json.dumps(command_json))
        self.socket.send(dump)

    def disconnect(self):
        self.socket.close()
