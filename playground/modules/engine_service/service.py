import functools
import threading
from time import sleep

import cetech.engine as engine
from cetech.consoleproxy import NanoSub


class EngineSubscriber(threading.Thread):
    def __init__(self, log_url, msg_url, on_msg, on_log, echo=True):
        super().__init__()

        self._run = False

        self._log_sub = NanoSub(log_url, echo=echo) if log_url is not None else None
        self._msg_sub = NanoSub(msg_url, echo=echo) if msg_url is not None else None
        self._on_log = on_log
        self._on_msg = on_msg

    def run(self):
        self._run = True

        self._log_sub.connect()
        self._msg_sub.connect()

        while self._run:
            msg = None

            if self._log_sub is not None:
                msg = self._log_sub.try_recive()
                if msg is not None:
                    self._on_log(msg)

            if self._msg_sub:
                msg = self._msg_sub.try_recive()
                if msg is not None:
                    self._on_msg(msg)

            if msg is None:
                sleep(0)

        self._log_sub.close()
        self._msg_sub.close()

    def close(self):
        self._run = False


class EngineService(object):
    def __init__(self, service_manager):
        self.service_manager = service_manager
        self.spawned_process = []
        self.instances = {}

        self.engine_bin_dir = "../bin"
        self.engine_external_dir = "../externals/build"

    def _log_proxy(self, instance_name, log):
        self.service_manager.publish("engine_service", "log", dict(instance_name=instance_name, log=log))

    def _pub_proxy(self, instance_name, pub):
        self.service_manager.publish("engine_service", "pub", dict(instance_name=instance_name, pub=pub))

    def close(self):
        self.api_killall()

    @property
    def _project_service(self):
        """
        :rtype: ProjectService
        """
        return self.service_manager.get_service_instance("project_service")

    def run(self):
        pass

    def api_call(self, instance_name, fce_name, **kwargs):
        if instance_name not in self.instances:
            return "Not ready"

        instance = self.instances[instance_name]

        if not instance.ready:
            return "Not ready"

        fce = getattr(instance.console_api, fce_name)
        return fce(**kwargs)

    def api_kill(self, name):
        if name in self.instances:
            instance = self.instances[name]
            instance.kill()

            del self.instances[name]

    def api_killall(self, dump=False):
        for name in self.instances.keys():
            self.api_kill(name)

    def api_run_release(self, name):
        if name in self.instances:
            self.instances[name].kill()

        instance = engine.Instance(name, self.engine_bin_dir, self.engine_external_dir)
        instance.run_release(self._project_service.build_dir)

        self.instances[name] = instance

    def api_run_develop(self, name, compile_=False, continue_=False, wait=False, daemon=False, wid=None,
                        core_dir=None, bootscript=None, bootpkg=None, lock=True, rpc_addr=None,
                        log_addr=None, pub_addr=None, push_addr=None):

        if name in self.instances:
            self.instances[name].kill()

        if log_addr or pub_addr:
            subscriber = EngineSubscriber(log_url=log_addr,
                                          on_log=functools.partial(self._log_proxy, name),
                                          msg_url=pub_addr,
                                          on_msg=functools.partial(self._pub_proxy, name),
                                          echo=False)
            subscriber.start()

        instance = engine.Instance(name, self.engine_bin_dir, self.engine_external_dir)
        instance.run_develop(
            self._project_service.build_dir,
            self._project_service.source_dir,
            compile_=compile_,
            continue_=continue_,
            wait=wait,
            daemon=daemon,
            wid=wid,
            core_dir=core_dir,
            bootscript=bootscript,
            bootpkg=bootpkg,
            lock=lock,
            rpc_addr=rpc_addr,
            log_addr=log_addr,
            pub_addr=pub_addr,
            push_addr=push_addr
        )

        self.instances[name] = instance
