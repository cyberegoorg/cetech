import glob
import logging
import os
from collections import OrderedDict

import yaml


class Manager(object):
    MOODULE_CONFIG_NAME = "modules.yaml"

    def __init__(self, logger=logging):
        self.logger = logger

        self.types = OrderedDict()
        self.loaded_modules = {}

    def register_module_type(self, type_name, fce):
        self.types[type_name] = fce

    def load_in_path(self, path):
        types = {type_name: [] for type_name in self.types.keys()}

        for filename in glob.iglob(os.path.join(path, "**", Manager.MOODULE_CONFIG_NAME), recursive=True):
            with open(filename) as f:
                data = yaml.load(f.read())

            name = data['name']
            description = data['description']

            self.logger.info("Loading module '%s' from '%s'", name, filename)

            self.loaded_modules[name] = {'desc': description}

            if 'extensions' in data:
                extensions = data['extensions']

                if extensions is None:
                    continue

                for type_name in types.keys():
                    lst = extensions.get(type_name, [])
                    if not lst:
                        continue

                    for type_item in lst:
                        type_item['__filename__'] = filename

                    types[type_name].extend(lst)

        for k in self.types.keys():
            self.types[k](types[k])
