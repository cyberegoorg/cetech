from cetech.shared.proxy import ConsoleProxy


class ConsoleAPI(ConsoleProxy):
    def lua_execute(self, script):
        self.send_command('lua.execute', script=script)

    def compile_all(self):
        self.send_command('resource_compiler.compile_all')
