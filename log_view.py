#!/usr/bin/env python

""" CETech yaml log view script
"""

###########
# IMPORTS #
########################################################################################################################

import re
import sys
import argparse

import yaml

###########
# GLOBALS #
########################################################################################################################

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


########
# ARGS #
########################################################################################################################

ARGS_PARSER = argparse.ArgumentParser(description='CETech yaml log view script')

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
        "logfile",
        nargs='?',
        help='Log file',
        default='log.yaml',
        type=argparse.FileType('r'))

ARGS_PARSER.add_argument(
        "-o", "--out",
        nargs='?',
        help='Output file',
        default=sys.stdout,
        type=argparse.FileType('w'))


###########
# PROGRAM #
########################################################################################################################
def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

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
        msg_re = re.compile(args.msg)
    else:
        msg_re = None

    log_yaml = yaml.load_all(args.logfile.read())

    out = args.out
    for entry in log_yaml:
        if entry['level'] not in level_mask:
            continue

        if where_re and not where_re.match(entry['where']):
            continue

        if msg_re and not msg_re.match(entry['msg']):
            continue

        out.write(PRINT_TEMPLATE.format(**entry))


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
