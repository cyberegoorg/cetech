import argparse
import json
import os


class ModuleBase(object):
    def __init__(self):
        super(ModuleBase, self).__init__()

        self.parser = argparse.ArgumentParser()
        self.parser.add_argument("--data-dir", type=str, help="data dir")
        self.args = self.parser.parse_args()
        self.data_dir = self.args.data_dir

        self.settings = None

        self.load_setting()

    def load_setting(self):
        if self.data_dir:
            self.settings = json.load(
                open(os.path.join(self.data_dir, 'config.json'), 'r')
            )

