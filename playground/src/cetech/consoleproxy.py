import logging

import msgpack
import nanomsg
from nanomsg import Socket, REQ, PUB, SUB, SUB_SUBSCRIBE, NanoMsgAPIError


class NanoPub(object):
    def __init__(self, url, send_timeout=5000, echo=False, logger=logging):
        self.echo = echo
        self.logger = logger.getLogger("nanopub")

        self.url = url  # .encode()
        self.socket = Socket(PUB)

        self.socket.send_timeout = send_timeout

        super(NanoPub, self).__init__()

    def bind(self):
        self.logger.debug("Bind to %s", self.url)
        self.socket.bind(self.url)

    def close(self):
        self.logger.debug("Unbind from %s", self.url)
        self.socket.close()

    def publish(self, msg):
        dump = msgpack.packb(msg)

        if self.echo:
            self.logger.debug("Publish:%s: %s", self.url, msg)

        try:
            self.socket.send(dump)
        except NanoMsgAPIError as e:
            self.logger.error("Publish error: %s", e.msg)


class NanoSub(object):
    def __init__(self, url, echo=False, logger=logging):
        self.echo = echo
        self.logger = logger.getLogger("nanosub")

        self.url = url  #.encode()
        self.socket = Socket(SUB)
        self.socket.set_string_option(SUB, SUB_SUBSCRIBE, b'')

        super(NanoSub, self).__init__()

    def try_recive(self):
        try:
            data = self.socket.recv(flags=nanomsg.DONTWAIT)
        except nanomsg.NanoMsgAPIError as e:
            if e.errno != 11:
                self.logger.exception(e)
                raise

            return None

        unpack_msg = msgpack.unpackb(data, encoding='utf-8')

        if self.echo:
            self.logger.debug("Subscribe: %s", unpack_msg)

        return unpack_msg

    def connect(self):
        self.logger.debug("Connect to %s", self.url)
        self.socket.connect(self.url)

    def close(self):
        self.logger.debug("Disconnect from %s", self.url)
        self.socket.close()


class NanoReq(object):
    def __init__(self, url, recv_timeout=5000, send_timeout=5000, echo=False, logger=logging):
        self.echo = echo
        self.logger = logger.getLogger("nanoreq")

        self.url = url  #.encode()
        self.socket = Socket(REQ)

        self.socket.recv_timeout = recv_timeout
        self.socket.send_timeout = send_timeout

        super(NanoReq, self).__init__()

    def connect(self):
        self.logger.debug("Connect to %s", self.url)
        self.socket.connect(self.url)

    def close(self):
        self.logger.debug("Disconnect from %s", self.url)
        self.socket.close()

    def send(self, msg):
        dump = msgpack.packb(msg)

        if self.echo:
            self.logger.debug("Send:%s: %s", self.url, msg)

        try:
            self.socket.send(dump)
        except NanoMsgAPIError as e:
            self.logger.error("Send error: %s", e.msg)

        try:
            recv = self.socket.recv()
            unpack_msg = msgpack.unpackb(recv, encoding='utf-8')

            if self.echo:
                self.logger.debug("Recive: %s", unpack_msg)

            if 'error' not in unpack_msg:
                return unpack_msg['response']

            raise ValueError(unpack_msg['error'])

        except NanoMsgAPIError as e:
            self.logger.error("Recive error: %s", e.msg.decode("utf-8"))


class ConsoleProxy(NanoReq):
    def send_command(self, cmd_name, **kwargs):
        command = {
            "name": cmd_name,
            "args": kwargs
        }
        return self.send(command)
