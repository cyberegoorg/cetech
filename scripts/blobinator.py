import glob
import os
import re
from collections import OrderedDict

import jinja2
import yaml

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir))

ELEMENT_RE = re.compile(r"(?P<type>.*) (?P<name>\w*)(\[(?P<count>\w*|\*)\])?")
TYPE_RE = re.compile(r"(?P<type>\w*)(\[(?P<count>\d*|\*)\])?")


def parse_blob(blob, jinja_env):
    elements = OrderedDict()
    values = blob['values']  # type: list[str]
    blob_name = blob['name']  # type: str
    guard_prefix = blob['guard_prefix']  # type: str
    includes = blob['includes']  # type: str

    prev_item_name = None
    first_array_idx = None
    for idx, v in enumerate(values):
        element = ELEMENT_RE.match(v)

        full_type = element['type']
        name = element['name']
        count = element['count']

        is_array = count is not None
        is_first_array = False
        if is_array and first_array_idx is None:
            first_array_idx = idx
            is_first_array = True

        element = TYPE_RE.match(full_type)
        type = element['type']
        type_count = element['count']

        if prev_item_name is not None:
            prev_item = elements[prev_item_name]
        else:
            prev_item = None

        prev_item_name = name

        elements[name] = dict(
            name=name,
            type=type,
            full_type=full_type,
            type_count=type_count,
            count=count,
            is_first_array=is_first_array,
            is_array=is_array,
            prev_item=prev_item
        )

    result = jinja_env.get_template("blob.h").render(
        name=blob_name,
        elements=elements,
        guard_prefix=guard_prefix,
        includes=includes
    )

    return result


def do_yaml(input_yaml, output_dir, jinja_env):
    blobs = input_yaml['blobs']  # type: list[]

    for blob in blobs:
        blob_name = blob['name']
        result = parse_blob(blob, jinja_env)

        with open(os.path.join(output_dir, "%s_blob.h" % blob_name), "w") as f:
            f.write(result)


def main():
    jinja_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(os.path.join(ROOT_DIR, "scripts", "templates")),
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=False
    )

    p = os.path.join(ROOT_DIR, "engine", "src", "**", "*_blob.yml")
    for filename in glob.iglob(p, recursive=True):
        with open(filename) as f:
            data = yaml.load(f.read())
            do_yaml(data, os.path.dirname(filename), jinja_env)


if __name__ == '__main__':
    main()
