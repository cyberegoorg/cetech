class FrontendService(object):
    SERVICE_NAME = "frontend_service"

    def __init__(self, service_manager):
        self.service_manager = service_manager

    def api_publish(self, service_name, msg_type, msg):
        self.service_manager.publish(service_name, msg_type, msg)

    def run(self):
        pass

    def close(self):
        pass
