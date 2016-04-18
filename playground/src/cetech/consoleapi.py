from cetech.consoleproxy import ConsoleProxy


class ConsoleAPI(ConsoleProxy):
    def wait(self):
        return self.send_command('wait')

    def lua_execute(self, script):
        return self.send_command('lua.execute', script=script)

    def compile_all(self):
        return self.send_command('resource_compiler.compile_all')

    def resize(self, w, h):
        return self.send_command('renderer.resize', width=w, height=h)

    def reload_all(self, types):
        return self.send_command('resource_manager.reload_all', types=types)

    def quit(self):
        return self.send_command('lua.execute', script="Application.Quit();")
