import os

os.sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from gi.repository import Gtk, GLib
from core.cetech.api import ConsoleAPI
from core.module.module_base import ModuleBase


class Main(ModuleBase):
    def __init__(self):
        super(Main, self).__init__()

        path = os.path.dirname(__file__)
        builder = Gtk.Builder()
        builder.add_from_file(os.path.join(path, "mainwindow.glade"))
        builder.connect_signals(self)

        self.window = builder.get_object("mainwindow")
        self.command_line_edit = builder.get_object("command_line_edit")
        self.log_store = builder.get_object("logstore")
        self.command_line_store = builder.get_object("commandlinestore")

        self.console_port = 0
        self.parse_setting()

        self.api = ConsoleAPI('localhost', self.console_port)
        self.api.register_on_log_handler(self.on_log)
        #self.api.register_on_lua_return(self.on_lua_return)

        GLib.idle_add(self.api.tick)

    def parse_setting(self):
        if self.settings:
            self.console_port = self.settings['cvars']['console_port']

    def main(self):
        self.api.start()

        self.window.show_all()
        Gtk.main()

    def on_log(self, level, where, msg):
        level_2_color = {
            'I': 'light blue',
            'D': 'light green',
            'W': 'light yellow',
            'E': 'red',
        }

        level_2_text = {
            'I': 'Info',
            'D': 'Debug',
            'W': 'Warning',
            'E': 'Error',
        }

        level_2_icon = {
            'I': 'gtk-dialog-info',
            'D': 'gtk-dialog-question',
            'W': 'gtk-dialog-warning',
            'E': 'gtk-dialog-error',
        }

        self.log_store.prepend([level_2_text[level], where, msg, level_2_color[level], level_2_icon[level]])

        if where == 'lua':
            self.command_line_store.append([msg, ])

    def on_lua_return(self, ret):
        self.command_line_store.append([str(ret), ])

    def on_execute(self, *args):
        script = self.command_line_edit.get_text()
        self.command_line_store.append([script])

        # script = "return to_json(%s)" % script
        self.command_line_edit.set_text("")
        self.api.lua_execute(script=script)

    def on_quit(self, *args):
        self.api.disconnect()
        Gtk.main_quit()


if __name__ == '__main__':
    Main().main()