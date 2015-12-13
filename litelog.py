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

import time
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
        "--sqlite-db",
        nargs='?',
        help='SQLite file',
        default='litelog.db',
        type=str)


###########
# PROGRAM #
########################################################################################################################
def crate_table(conn):
    sql = """
    CREATE TABLE IF NOT EXISTS log (
        time INTEGER  NOT NULL,
        level TEXT NOT NULL,
        worker INTEGER NOT NULL,
        wheree TEXT NOT NULL,
        msg TEXT NOT NULL
    );
    """
    conn.execute(sql)
    conn.commit()


def insert_log(conn, time, level, worker, where, msg):
    sql = """
    INSERT INTO log VALUES (?, ?, ?, ?, ?);
    """
    conn.execute(sql, (time, level, worker, where, msg))
    conn.commit()


def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

    conn = sqlite3.connect(args.sqlite_db)
    crate_table(conn)

    socket = nanomsg.Socket(nanomsg.SUB)
    socket.set_string_option(nanomsg.SUB, nanomsg.SUB_SUBSCRIBE, b'#log')
    socket.connect(args.url.encode())

    while True:
        try:
            msg = socket.recv(flags=nanomsg.DONTWAIT)

            msg_yaml = yaml.load(msg)
            insert_log(conn, **msg_yaml)

        except nanomsg.NanoMsgAPIError as e:
            if e.errno == nanomsg.EAGAIN:
                continue

            raise

        time.sleep(0)

    socket.close()

########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
