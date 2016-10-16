from playground.core.modules import PlaygroundModule


class CompilerModule(PlaygroundModule):
    Name = "compiler"

    def __init__(self, module_manager):
        super(CompilerModule).__init__()
        self.init_module(module_manager)
        self.engine = None

    def compile_all(self):
        self.engine.console_api.compile_all()

    def on_open_project(self, project):
        self.engine = project.run_develop("compiler", compile_=True, continue_=True, bootscript="playground/empty_boot",
                                          bootpkg="playground/empty", daemon=True, port=5586)

    def on_close_project(self):
        self.engine.console_api.quit()
        self.engine.console_api.disconnect()
