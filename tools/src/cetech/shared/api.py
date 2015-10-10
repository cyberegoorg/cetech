from cetech.shared.proxy import ConsoleProxy


class ConsoleAPI(ConsoleProxy):
    def lua_execute(self, script):
        self.send_command('lua.execute', script=script)

    def compile_all(self):
        self.send_command('resource_compiler.compile_all')

    def autocomplete_list(self):
        self.lua_execute("""
        local t = {}
        local i = 0
        for class_name,class in pairs(_G) do
            if type(class) == 'table' then
                for function_name,fce in pairs(class) do
                    if type(fce) == 'function' then
                        t[tostring(i)] = class_name .. '.' .. function_name
                        i = i + 1
                    end
                end
            end
        end
        Application.console_send({
            type = "autocomplete_list",
            list = t
        })
  """)

