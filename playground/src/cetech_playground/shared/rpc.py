import random

import functools

from cetech import NanoReq


class Client(object):
    def __init__(self, **kwargs):
        self.req_rep = NanoReq(**kwargs)

    def connect(self):
        self.req_rep.connect()

    def close(self):
        self.req_rep.close()

    def partial_call_service(self, service_name):
        return functools.partial(self.call_service, service_name)

    def call_service(self, service, cmd_name, **kwargs):
        msg_id = random.random()

        command = {
            "service": service,
            "name": cmd_name,
            "args": kwargs,
            "id": msg_id
        }

        return self.req_rep.send(command)
