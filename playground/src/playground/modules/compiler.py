from playground.core.modules import PlaygroundModule


class CompilerModule(PlaygroundModule):
    def __init__(self, module_manager):
        super(CompilerModule).__init__()
        self.project_manager = None
        self.init_module(module_manager)
        self.modules_manager.new_common(self, 'compiler')
        self.engine = None

    def compile_all(self):
        self.engine.console_api.compile_all()

    def open_project(self, project):
        self.project_manager = project
        self.engine = project.run_develop("compiler", compile_=True, continue_=True, bootscript="playground/empty_boot",
                                          bootpkg="playground/empty", daemon=True, port=5586)

    def close_project(self):
        self.engine.console_api.quit()
        self.engine.console_api.disconnect()
