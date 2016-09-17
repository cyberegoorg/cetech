#!/usr/bin/env python

""" CETech log -> sql dumper
"""

###########
# IMPORTS #
########################################################################################################################
import argparse
import ctypes
import os
import platform
import sqlite3
from argparse import RawTextHelpFormatter

import yaml

###########
# GLOBALS #
########################################################################################################################
ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)))

_platform = platform.system().lower()
LIB_DIR = os.path.join(ROOT_DIR,
                       'externals',
                       'build',
                       "%s%s" % (_platform if _platform != 'darwin' else 'osx', platform.architecture()[0][0:2]),
                       'dynlib')
_original_load = ctypes.cdll.LoadLibrary
ctypes.cdll.LoadLibrary = lambda x: _original_load(os.path.join(LIB_DIR, x))

import nanomsg

##########
# CONFIG #
########################################################################################################################

#######
# SQL #
########################################################################################################################
CREATE_SQL = """
CREATE TABLE IF NOT EXISTS log (
    time INTEGER  NOT NULL,
    level TEXT NOT NULL,
    worker INTEGER NOT NULL,
    wheree TEXT NOT NULL,
    msg TEXT NOT NULL
);
"""

INSERT_SQL = """
INSERT INTO log VALUES (?, ?, ?, ?, ?);
"""

########
# ARGS #
########################################################################################################################
ARGS_PARSER = argparse.ArgumentParser(description='CETech log -> sql dumper', formatter_class=RawTextHelpFormatter)

ARGS_PARSER.add_argument(
    "-u", "--url",
    help='Connection url',
    default='ws://localhost:5556',
    type=str)

ARGS_PARSER.add_argument(
    "-d", "--sqlite-db",
    nargs='?',
    help='SQLite file',
    default='litelog.db',
    type=str)

ARGS_PARSER.add_argument(
    "-i", "--import",
    help='Log file to import',
    dest='importt',
    type=str)


###########
# PROGRAM #
########################################################################################################################
def crate_table(conn):
    conn.execute(CREATE_SQL)
    conn.commit()


def insert_log(conn, time, level, worker, where, msg):
    conn.execute(INSERT_SQL, (time, level, worker, where, msg))
    conn.commit()


def insert_many_log(conn, logs):
    conn.executemany(INSERT_SQL, logs)
    conn.commit()


def import_log(conn, filename):
    with open(filename, 'r') as f:
        log_yaml = yaml.load_all(f.read())

    insert_many_log(conn, [(x['time'], x['level'], x['worker'], x['where'], x['msg'],) for x in log_yaml])


def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

    conn = sqlite3.connect(args.sqlite_db)
    crate_table(conn)

    if args.importt:
        import_log(conn, args.importt)

    else:
        socket = nanomsg.Socket(nanomsg.SUB)
        try:
            socket.set_string_option(nanomsg.SUB, nanomsg.SUB_SUBSCRIBE, b'#log')
            socket.connect(args.url.encode())

            while True:
                msg = socket.recv()
                msg_yaml = yaml.load(msg)
                insert_log(conn, **msg_yaml)

        except Exception:
            socket.close()
            raise


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
