#!/usr/bin/env python

""" CETech yaml log view script
"""

###########
# IMPORTS #
########################################################################################################################
import os
import re
import sys
import argparse
from argparse import RawTextHelpFormatter

import yaml

###########
# GLOBALS #
########################################################################################################################
ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)))

##########
# CONFIG #
########################################################################################################################
with open(os.path.join(ROOT_DIR, 'logformat.yaml'), 'r') as f:
    CONFIG = yaml.load(f.read())

TEMPLATE_ARGS = CONFIG['args']
TEMPLATE_ARGS_HELP = '\n\t'.join(["{%s} = %s" % (k, v) for k, v in TEMPLATE_ARGS.items()])

PRINT_TEMPLATE = CONFIG['template'][:-1]
LEVELS = CONFIG['levels']['types']

COLORS = {
    'red': "\x1B[31m",
    'green': "\x1B[32m",
    'yellow': "\x1B[33m",
    'blue': "\x1B[34m",
    'reset': "\033[0m"
}

LEVEL_TO_COLOR = {}
for k, v in CONFIG['colors'].items():
    if k == 'arg':
        continue

    LEVEL_TO_COLOR[k] = "%s%%s%s" % (COLORS[v], COLORS['reset'])

########
# ARGS #
########################################################################################################################
ARGS_PARSER = argparse.ArgumentParser(description='CETech yaml log view script', formatter_class=RawTextHelpFormatter)

ARGS_PARSER.add_argument(
        "logfile",
        nargs='?',
        help='Log file. \'-\' for stdin',
        default='log.yaml',
        type=argparse.FileType('r'))

for v in LEVELS:
    ARGS_PARSER.add_argument(
            "--%s" % v,
            help='Show %s msg' % v,
            action='store_true')

for k, v in TEMPLATE_ARGS.items():
    ARGS_PARSER.add_argument(
            "--%s" % k,
            help='%s regexp' % v,
            type=str)

ARGS_PARSER.add_argument(
        "-t", "--template",
        help='Output template.\n  Variables:\n\t%s' % TEMPLATE_ARGS_HELP,
        default=PRINT_TEMPLATE,
        type=str)

ARGS_PARSER.add_argument(
        "-o", "--out",
        nargs='?',
        help='Output file',
        default=sys.stdout,
        type=argparse.FileType('w'))

ARGS_PARSER.add_argument(
        "--no-color",
        help='Disable colors',
        action='store_true')

ARGS_PARSER.add_argument(
        "--count",
        help='Print log entry count',
        action='store_true')


###########
# PROGRAM #
########################################################################################################################
def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

    if args.no_color:
        global LEVEL_TO_COLOR
        LEVEL_TO_COLOR = {k: "%s" for k, v in LEVEL_TO_COLOR.items()}

    any_level_mask = False
    for level in LEVELS:
        attr = getattr(args, level)
        if attr:
            any_level_mask = True
            break

    if any_level_mask:
        level_mask = set()

        for level in LEVELS:
            if getattr(args, level):
                level_mask.add(level)
    else:
        level_mask = LEVELS

    arg_re = {}
    for k, v in TEMPLATE_ARGS.items():
        if getattr(args, k):
            arg_re[k] = re.compile(getattr(args, k), re.MULTILINE | re.DOTALL)

    log_data = args.logfile.read()
    for c in COLORS:
        log_data = log_data.replace(c, '')

    log_yaml = yaml.load_all(log_data)

    out = args.out
    count = args.count
    entry_count = 0
    template_format = args.template.format
    out_write = out.write

    for entry in log_yaml:
        level = entry['level']
        if level not in level_mask:
            continue

        skip = False
        for k, v in arg_re.items():
            if not v.match(str(entry[k])):
                skip = True
                break

        if skip:
            continue

        if count:
            entry_count += 1
            continue

        out_write(LEVEL_TO_COLOR[level] % template_format(**entry))

    if count:
        out_write = entry_count
        print("count: %s" % entry_count)


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
