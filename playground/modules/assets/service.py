import functools

import logging


class AssetService(object):
    SERVICE_NAME = "asset_service"

    CLICK_MSG_TYPE = 'click'
    DOUBLE_CLICK_MSG_TYPE = 'dclick'

    def __init__(self, service_manager, logger=logging):
        self.logger = logger.getLogger("asset_service")
        self.service_manager = service_manager
        self.publish = functools.partial(self.service_manager.publish, self.SERVICE_NAME, "event")

    @property
    def _engine_service(self):
        """
        :rtype: EngineService
        """
        return self.service_manager.get_service_instance("engine_service")

    def run(self):
        self._engine_service.api_run_develop(
            name="asset_compiler",
            compile_=True,
            continue_=True,
            daemon=True,
            lock=True,
            rpc_addr="ipc:///tmp/cetech_asset_compiler_rpc.ipc",
            log_addr="ipc:///tmp/cetech_asset_compiler_log.ipc",
            pub_addr="ipc:///tmp/cetech_asset_compiler_pub.ipc",
            push_addr="ipc:///tmp/cetech_asset_compiler_push.ipc",
            bootpkg="playground/empty",
            bootscript="playground/empty_boot")

    def close(self):
        self._engine_service.api_kill("asset_compiler")

    def api_asset_click(self, path, name, type):
        self.publish(dict(msg_type=self.CLICK_MSG_TYPE, path=path, name=name, type=type))

    def api_asset_dclick(self, path, name, type):
        self.publish(dict(msg_type=self.DOUBLE_CLICK_MSG_TYPE, path=path, name=name, type=type))

    def api_compile_all(self):
        self.logger.debug("Compile all assets")
        self._engine_service.api_call(instance_name="asset_compiler", fce_name="compile_all")
