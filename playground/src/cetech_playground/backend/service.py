import os
from collections import defaultdict


class ServiceManager(object):
    def __init__(self, server):
        """
        :type server: playground.backend.server.Server
        """
        self.server = server

        self.service_instances = {}
        self.service_api = {}
        self._service_subscribers = defaultdict(list)

    def close(self):
        for service in self.service_instances.values():
            service.close()

    def run(self):
        for service in self.service_instances.values():
            service.run()

    def get_service_instance(self, name):
        return self.service_instances[name]

    def get_service_api(self, name):
        return self.service_api[name]

    def publish(self, service_name, msg_type, msg):
        self.server.publish(dict(msg_type=msg_type, service=service_name, msg=msg))

        for clb in self._service_subscribers[service_name]:
            clb(msg)

    def subscribe_service(self, service_name, subscribe_clb):
        self._service_subscribers[service_name].append(subscribe_clb)

    def load_services(self, services):
        for service in services:
            api = service['api']
            name = service['name']
            filename = service['__filename__']
            script_file = service['script_file']
            script_class = service['script_class']

            script_file_path = os.path.join(os.path.dirname(filename), script_file)
            with open(script_file_path) as f:
                compiled_service = compile(f.read(), script_file_path, "exec")

            service_global = {}
            exec(compiled_service, service_global)

            service_instance = service_global[script_class](self)

            self.service_instances[name] = service_instance
            self.service_api[name] = api

    def load_statics(self, statics):
        for static in statics:
            filename = static['__filename__']

            prefix = "/modules/%s" % static['prefix']
            path = os.path.abspath(os.path.join(os.path.dirname(filename), "static"))

            self.server.add_static(prefix=prefix, path=path)

    def register_to_module_manager(self, module_manager):
        module_manager.register_module_type("service", self.load_services)
        # module_manager.register_module_type("statics", self.load_statics)
