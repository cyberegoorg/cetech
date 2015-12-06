#!/usr/bin/env python

""" CETech yaml log view script
"""

###########
# IMPORTS #
########################################################################################################################

import re
import sys
import argparse
from argparse import RawTextHelpFormatter

import yaml

###########
# GLOBALS #
########################################################################################################################
TEMPLATE_ARGS = {
    'level': 'Log level',
    'where': 'Where',
    'time': 'Time',
    'worker': 'Worker ID',
    'msg': 'Message'
}

TEMPLATE_ARGS_HELP = '\n\t'.join(["{%s} = %s" % (k, v) for k, v in TEMPLATE_ARGS.items()])

PRINT_TEMPLATE = "---\n" \
                 "level: {level}\n" \
                 "where: {where}\n" \
                 "time: {time}\n" \
                 "worker: {worker}\n" \
                 "msg: |\n" \
                 "  {msg}"

##########
# CONFIG #
########################################################################################################################

COLOR_RED = "\x1B[31m"
COLOR_GREEN = "\x1B[32m"
COLOR_YELLOW = "\x1B[33m"
COLOR_BLUE = "\x1B[34m"
COLOR_RESET = "\033[0m"

LEVEL_TO_COLOR = {
    'info': COLOR_BLUE + "%s" + COLOR_RESET,
    'error': COLOR_RED + "%s" + COLOR_RESET,
    'debug': COLOR_GREEN + "%s" + COLOR_RESET,
    'warning': COLOR_YELLOW + "%s" + COLOR_RESET,
}

########
# ARGS #
########################################################################################################################

ARGS_PARSER = argparse.ArgumentParser(description='CETech yaml log view script', formatter_class=RawTextHelpFormatter)

ARGS_PARSER.add_argument(
        "-i", "--info",
        help='Show info msg',
        action='store_true')

ARGS_PARSER.add_argument(
        "-e", "--error",
        help='Show error msg',
        action='store_true')

ARGS_PARSER.add_argument(
        "-d", "--debug",
        help='Show debug msg',
        action='store_true')

ARGS_PARSER.add_argument(
        "-w", "--warn",
        help='Show warning msg',
        action='store_true')

ARGS_PARSER.add_argument(
        "--where",
        help='Where regexp',
        type=str)

ARGS_PARSER.add_argument(
        "--msg",
        help='Message regexp',
        type=str)

ARGS_PARSER.add_argument(
        "-t", "--template",
        help='Output template. Variables:\n\t%s' % TEMPLATE_ARGS_HELP,
        default=PRINT_TEMPLATE,
        type=str)

ARGS_PARSER.add_argument(
        "-o", "--out",
        nargs='?',
        help='Output file',
        default=sys.stdout,
        type=argparse.FileType('w'))

ARGS_PARSER.add_argument(
        "logfile",
        nargs='?',
        help='Log file. \'-\' for stdin',
        default='log.yaml',
        type=argparse.FileType('r'))

ARGS_PARSER.add_argument(
        "--no-color",
        help='Disable colors',
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

    if not (args.info or args.error or args.debug or args.warn):
        level_mask = {"info", "debug", "warning", "error"}

    else:
        level_mask = set()
        if args.info:
            level_mask.add("info")

        if args.debug:
            level_mask.add("debug")

        if args.warn:
            level_mask.add("warning")

        if args.error:
            level_mask.add("error")

    if args.where:
        where_re = re.compile(args.where)
    else:
        where_re = None

    if args.msg:
        msg_re = re.compile(args.msg, re.MULTILINE)
    else:
        msg_re = None

    log_yaml = yaml.load_all(args.logfile.read())

    out = args.out
    for entry in log_yaml:
        level = entry['level']
        if level not in level_mask:
            continue

        if where_re and not where_re.match(entry['where']):
            continue

        if msg_re and not msg_re.match(entry['msg']):
            continue

        out.write(LEVEL_TO_COLOR[level] % args.template.format(**entry).replace('\\n', '\n'))


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
