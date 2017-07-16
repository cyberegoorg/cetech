import argparse
import glob
import os
import re

import jinja2
from tabulate import tabulate

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir))

MODULE_RE = re.compile(r"(?P<module_comment>(--!.*\n)*)(?P<module_name>[\S]*)[ ]*=[ \S]*{}")
FCE_RE = re.compile(
    r"(?P<function_comments>(--!.*\n)*)^[\S]*function[\s]*(?P<function_name>[\S]*)\((?P<function_args>[\w,\s)]*)\)[\S\s]",
    re.MULTILINE
)

ARGS_PARSER = argparse.ArgumentParser(description='Doc generator')
ARGS_PARSER.add_argument(
    "--template_dir",
    default=os.path.join(ROOT_DIR, "scripts", "dockemall"),
    help='Template dir')

ARGS_PARSER.add_argument(
    "-t", "--template",
    default="module.md",
    help='Template')

ARGS_PARSER.add_argument(
    '-f,', '--file',
    type=argparse.FileType('r'))

ARGS_PARSER.add_argument(
    "-d", "--input-dir",
    help='Input dir')

ARGS_PARSER.add_argument(
    "-o", "--output",
    type=argparse.FileType('w'),
    help='Output filename')


def render_md(modules, jinja_env, template):
    result = jinja_env.get_template(template).render(modules=modules)
    return result


def parse_module_comments(comments):
    comments = comments.replace("--! ", "").replace("--!", "").split("\n")

    module_comment = []
    special_comment = []
    for l in comments:
        # l = l.strip()
        if l.startswith("\\"):
            special_comment.append(l)
        else:
            module_comment.append(l)

    module_comment = '\n'.join(module_comment)

    return module_comment


def parse_fce_comments(comments):
    comments = comments.replace("--! ", "").replace("--!", "").split("\n")

    fce_comment = []
    special_comment = []
    for l in comments:
        # l = l.strip()
        if l.startswith("\\"):
            special_comment.append(l)
        else:
            fce_comment.append(l)

    fce_comment = '\n'.join(fce_comment)

    return_dsc = None
    args_dsc = []

    for s in special_comment:
        line_split = s.split(" ")
        comment_type = line_split[0]

        if comment_type == '\\return':
            return_dsc = dict(type=line_split[1], dsc=' '.join(line_split[2:]))

        elif comment_type == '\\param':
            param_name = line_split[1]
            param_type = line_split[2]
            param_dsc = ' '.join(line_split[3:])

            args_dsc.append(dict(name=param_name, type=param_type, dsc=param_dsc))

    d = dict(comments=fce_comment)

    if return_dsc is not None:
        d['return_dsc'] = return_dsc

    if len(args_dsc) > 0:
        d['args_dsc'] = args_dsc
        ##d['args_tabulate'] = tabulate_args_dsc(args_dsc)

    return d


def tabulate_args_dsc(args_dsc):
    headers = ['name', 'type', 'description']
    tbl = []

    for name, dsc in args_dsc.items():
        tbl.append([name, dsc['type'], dsc['dsc']])

    return tabulate(tbl, headers, tablefmt="pipe")


def parse_lua(file, modules):
    functions = []
    file_content = file.read()

    module_match = MODULE_RE.findall(file_content)
    for m in module_match:
        module_name = m[2]
        comments = m[0]
        comments = parse_module_comments(comments)

        fce_match = FCE_RE.findall(file_content)

        for m in fce_match:
            function_name = m[2].strip()
            args = [x.strip() for x in m[3].split(',')]
            if args[0] == '':
                args = []

            comments_dict = parse_fce_comments(m[0])

            fce_dict = dict(
                name=function_name,
                args=args,
            )

            fce_dict.update(comments_dict)

            functions.append(fce_dict)

        modules.append(dict(name=module_name, comments=comments, fce=functions))


def main(args=None):
    args = ARGS_PARSER.parse_args(args=args)

    jinja_env = jinja2.Environment(
        loader=jinja2.FileSystemLoader(args.template_dir),
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=False
    )

    modules = []
    result = ""

    if args.file:
        parse_lua(args.file, modules=modules)
        result = render_md(modules=modules, jinja_env=jinja_env, template=args.template)

    if args.input_dir:
        for filename in glob.iglob(os.path.join(args.input_dir, "**", "*.lua"), recursive=True):
            with open(filename) as f:
                parse_lua(f, modules=modules)

    elif args.file:
        parse_lua(args.file, modules=modules)

    result = render_md(modules=modules, jinja_env=jinja_env, template=args.template)

    if args.output:
        args.output.write(result)

    else:
        print(result)


if __name__ == '__main__':
    main()
