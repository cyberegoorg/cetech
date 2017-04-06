from cetech.playground.frontend import CETechWiget


class LevelEditorView(CETechWiget):
    def __init__(self, frontend, parent=None):
        self.frontend = frontend
        self.rpc = frontend.rpc

        self.frontend.subscribe_service("asset_service", self._subcribe_asset_browser)

        super(LevelEditorView, self).__init__(self.rpc, "level_view", parent=parent)

        self.created = False

    def _subcribe_asset_browser(self, msg):
        msg_type = msg['msg_type']

        name = msg['name']
        type = msg['type']

        if msg_type == 'dclick' and type == 'level':
            self.rpc.call_service("engine_service", "call", instance_name=self.instance_name, fce_name="lua_execute",
                                  script="Editor:load_level(\"%s\")" % name)

    def showEvent(self, event):
        if not self.created:
            self.rpc.call_service("engine_service", "run_develop",
                                  name="level_view",
                                  compile_=True, continue_=True,
                                  lock=True, wid=int(self.wid),
                                  rpc_addr="ipc:///tmp/cetech_level_editor_rpc.ipc",
                                  log_addr="ipc:///tmp/cetech_level_editor_log.ipc",
                                  pub_addr="ipc:///tmp/cetech_level_editor_pub.ipc",
                                  push_addr="ipc:///tmp/cetech_level_editor_push.ipc",
                                  bootscript="playground/leveleditor_boot")
            self.created = True

        event.accept()

    def closeEvent(self, event):
        self.rpc.call_service("engine_service", "kill", name="level_view")
        event.accept()