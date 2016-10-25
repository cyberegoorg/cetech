#!/usr/bin/env python

""" CETech log grep
"""

###########
# IMPORTS #
########################################################################################################################
import argparse
import os
import sqlite3
import tabulate
from argparse import RawTextHelpFormatter

###########
# GLOBALS #
########################################################################################################################
from IPython.lib.inputhook import _stdin_ready_other

ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)))

##########
# CONFIG #
########################################################################################################################

#######
# SQL #
########################################################################################################################

########
# ARGS #
########################################################################################################################
ARGS_PARSER = argparse.ArgumentParser(description='CETech log grep', formatter_class=RawTextHelpFormatter)

ARGS_PARSER.add_argument(
    "-d", "--sqlite-db",
    nargs='?',
    help='SQLite file',
    default='log.db',
    type=str)

ARGS_PARSER.add_argument(
    "-w", "--where",
    nargs='?',
    help='where sql expression ex.: -w "= \'application\'" ')

ARGS_PARSER.add_argument(
    "-l", "--level",
    help='Log level sql expression ex.: -l "= \'error\'"',
    default='all')

ARGS_PARSER.add_argument(
    "-a", "--all",
    action='store_true',
    help='all sessions')

ARGS_PARSER.add_argument(
    "-c", "--count",
    action='store_true',
    help='return log row count')


###########
# PROGRAM #
########################################################################################################################

def get_logs(conn, where=None, level=None, last_session=True, count=False):
    if not count:
        header = ["level", "where", "message"]
        base_sql = """
        SELECT level, wheree, msg FROM log WHERE 1
        """

    else:
        header = ["count"]
        base_sql = """
        SELECT count(1) FROM log WHERE 1
        """

    if last_session:
        base_sql = "%s AND session_id = (SELECT max(session_id) FROM log)" % (base_sql)

    if where is not None:
        base_sql = "%s AND wheree %s" % (base_sql, where)

    if level != "all":
        base_sql = "%s AND level %s" % (base_sql, level)

    res = conn.execute(base_sql)

    print(tabulate.tabulate(res, headers=header, tablefmt="plain"))


def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)
    conn = sqlite3.connect(args.sqlite_db)

    get_logs(conn,
             where=args.where,
             level=args.level,
             last_session=not args.all,
             count=args.count)


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
