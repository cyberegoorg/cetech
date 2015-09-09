from .proxy import ConsoleProxy


class ConsoleAPI(ConsoleProxy):
    def lua_execute(self, script):
        self.send_command('lua.execute', script=script)
