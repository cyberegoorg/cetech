from cetech.consoleproxy import ConsoleProxy


class ConsoleAPI(ConsoleProxy):
    def wait(self):
        return self.send_command('wait')

    def lua_execute(self, script):
        return self.send_command('lua_system.execute', script=script)

    def compile_all(self):
        return self.lua_execute("cetech.ResourceManager.compile_all()")

    def reload_all(self):
        return self.lua_execute("cetech.ResourceManager.reload_all()")

    def resize(self, w, h):
        return self.send_command('renderer.resize', width=w, height=h)

    def quit(self):
        return self.lua_execute(script="cetech.Application.quit()")
