from cetech.consoleproxy import ConsoleProxy


class ConsoleAPI(ConsoleProxy):
    def wait(self):
        self.send_command('wait')
        pass

    def lua_execute(self, script):
        self.send_command('lua.execute', script=script)

    def compile_all(self):
        self.send_command('resource_compiler.compile_all')

    def resize(self, w, h):
        self.send_command('renderer.resize', width=w, height=h)
        pass

    def reload_all(self, types):
        self.send_command('resource_manager.reload_all', types=types)
        pass

    def quit(self):
        self.send_command_norcv('lua.execute', script="Application.Quit();")
