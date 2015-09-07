#!/usr/bin/env python3

import os
import subprocess
import yaml
from gi.repository import Gtk, GLib


class Main(object):
    def __init__(self):
        super(Main, self).__init__()
        self.path = os.path.dirname(__file__)
        builder = Gtk.Builder()
        builder.add_from_file(os.path.join(self.path, "mainwindow.glade"))
        builder.connect_signals(self)

        self.window = builder.get_object("mainwindow")
        self.project_dir_edit = builder.get_object("project_dir_edit")
        self.debug_tools_grid = builder.get_object("debug_tools_grid")
        self.main_box = builder.get_object("main_box")

        self.category_grids = {}
        self.find_modules()

    def run_module(self, name):
        subprocess.Popen(
            "python %s --data-dir %s" % (
                os.path.join(self.path, "..", "modules", "%s" % name, 'main.py'),
                self.project_dir_edit.get_text()
            ),
            shell=True
        )

    def find_modules(self):
        modules_dir = os.path.join(self.path, "..", 'modules')

        for filename in os.listdir(modules_dir):
            module_filename = os.path.join(os.path.join(modules_dir, filename), "module.yaml")

            if os.path.exists(module_filename):
                with open(module_filename, 'r') as f:
                    info = yaml.load(f)

                    def gen_callback(name):
                        return lambda *args: self.run_module(name)

                    category = info['category']
                    btn = Gtk.Button(label=info['name'])
                    btn.connect("clicked", gen_callback(info['name'].lower()))

                    if category in self.category_grids:
                        grid = self.category_grids[category]
                    else:
                        frame = Gtk.Frame(label=category)
                        grid = Gtk.Grid(row_homogeneous=True, column_homogeneous=True)
                        frame.add(grid)
                        self.main_box.add(frame)
                        self.category_grids[category] = grid

                    grid.add(btn)

    def main(self):
        self.window.show_all()
        Gtk.main()

    def on_close(self, *args):
        Gtk.main_quit()

    def on_select_project_dir(self, *args):
        dialog = Gtk.FileChooserDialog("Please choose a project folder", self.window,
                                       Gtk.FileChooserAction.SELECT_FOLDER,
                                       (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                        "Select", Gtk.ResponseType.OK))
        dialog.set_default_size(800, 400)

        response = dialog.run()
        if response == Gtk.ResponseType.OK:
            self.project_dir_edit.set_text(dialog.get_filename())

        dialog.destroy()


def main():
    Main().main()

if __name__ == '__main__':
    main()