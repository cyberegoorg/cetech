import logging
import logging.config
import os
import signal
from asyncio import sleep

import msgpack
import nanomsg
from aiohttp import web

from cetech.consoleproxy import NanoPub
from playground.backend.service import ServiceManager
from playground.shared.modules import Manager

ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir, os.pardir)
MODULES_DIR = os.path.abspath(os.path.join(ROOT_DIR, "modules"))
CORE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir, os.pardir, os.pardir, os.pardir, "core"))


class Server(object):
    def __init__(self, logger=logging):
        self.logger = logger

        self.app = web.Application()

        self.app.router.add_static('/static/',
                                   path=str(os.path.join(ROOT_DIR, 'html')),
                                   name='static')

        self.app.on_startup.append(self.start_rpc_task)
        self.app.on_cleanup.append(self.close)
        self.app.on_cleanup.append(self.cleanup_rpc_task)

        self.modules_manager = Manager()
        self.service_manager = ServiceManager(self)
        self.service_manager.register_to_module_manager(self.modules_manager)

        self.modules_manager.load_in_path(MODULES_DIR)

        self.pub = NanoPub("ws://*:8889", echo=False)
        self.pub.bind()

        self.rpc_run = True
        self.rpc_quit = False

        self.service_manager.run()

        def close_handler(signum, frame):
            self.logger.info("Close by signal")
            self.app.shutdown()

        signal.signal(signal.SIGKILL, close_handler)
        signal.signal(signal.SIGTERM, close_handler)

    def publish(self, msg):
        self.pub.publish(msg)

    def _on_msg(self, msg):
        service_name = msg["service"]
        fce_name = msg["name"]
        fce_args = msg.get("args", dict())

        service_api = self.service_manager.get_service_api(service_name)
        if fce_name not in service_api:
            response = {
                "error": "invalid api function"
            }

        else:
            service = self.service_manager.get_service_instance(service_name)

            service_fce_name = "api_%s" % fce_name
            service_fce = getattr(service, service_fce_name)

            ret = service_fce(**fce_args)

            response = ret

        return response

    async def rpc_task(self, url, on_msg):
        rpc_socket = nanomsg.Socket(nanomsg.REP)
        rpc_socket.bind(url)

        self.rpc_run = True
        while not self.rpc_quit:
            try:
                data = rpc_socket.recv(flags=nanomsg.DONTWAIT)
            except nanomsg.NanoMsgAPIError as e:
                if e.errno == 11:
                    await sleep(0.001)
                    continue

                else:
                    self.logger.exception(e)
                    break

            unpack_msg = msgpack.unpackb(data, encoding='utf-8')
            call_id = unpack_msg["id"]

            response = {"id": call_id}

            try:
                ret = on_msg(unpack_msg)

                response.update({
                    "response": ret
                })

            except Exception as e:
                response.update({
                    "error": str(e)
                })

                self.logger.exception(e)

            rpc_socket.send(msgpack.packb(response))

        self.rpc_run = False
        self.rpc_quit = False
        rpc_socket.close()

    async def start_rpc_task(self, app):
        app['rpc_task'] = app.loop.create_task(self.rpc_task('ws://*:8888', self._on_msg))

    async def cleanup_rpc_task(self, app):
        self.rpc_quit = True
        await app['rpc_task']

    def run(self):
        web.run_app(self.app, shutdown_timeout=1)

    def close(self, app):
        self.service_manager.close()


def run():
    logging.info("start")
    server = Server(logger=logging)
    server.run()
