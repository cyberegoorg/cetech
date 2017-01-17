#!/usr/bin/env python

###########
# IMPORTS #
########################################################################################################################
import argparse
import ctypes
import os
import platform
import sys
from multiprocessing import Process

import yaml
from PyQt5 import QtCore

_platform = platform.system().lower()
if _platform == 'windows':
    sys.path.insert(0, 'C:\Python34\lib\site-packages')

print(sys.path)

import PyQt5
from PyQt5.QtCore import QDir, qInstallMessageHandler
from PyQt5.QtWidgets import QApplication
from PyQt5.uic import compileUiDir

###########
# GLOBALS #
########################################################################################################################
ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir)
PLAYGROUND_DIR = os.path.abspath(os.path.join(ROOT_DIR, 'playground'))

_platform = platform.system().lower()
LIB_DIR = os.path.join(ROOT_DIR,
                       'externals', 'build',
                       "%s%s" % (_platform if _platform != 'darwin' else 'osx', platform.architecture()[0][0:2]),
                       'release', 'lib')

if _platform == 'windows':
    QApplication.addLibraryPath(os.path.join(os.path.dirname(PyQt5.__file__), "plugins"))
    ctypes.windll.LoadLibrary(
        os.path.join(ROOT_DIR, 'externals', 'build', 'windows64', 'Release', 'lib', 'nanomsg.dll'))
    # ctypes.windll.LoadLibrary('nanomsg.dll')
    sys.path.insert(0, 'C:\Python34\lib\site-packages')

else:
    _original_load = ctypes.cdll.LoadLibrary
    ctypes.cdll.LoadLibrary = lambda x: _original_load(os.path.join(LIB_DIR, x))

sys.path.insert(0, os.path.join(PLAYGROUND_DIR, 'src'))
QDir.setCurrent(PLAYGROUND_DIR)

########
# ARGS #
########################################################################################################################
ARGS_PARSER = argparse.ArgumentParser(description='Playground', formatter_class=argparse.RawTextHelpFormatter)

ARGS_PARSER.add_argument(
    "--frontend",
    action='store_true',
    help='Run frontend',
)

ARGS_PARSER.add_argument(
    "--backend",
    action='store_true',
    help='Run backend',
)


########
# MAIN #
########################################################################################################################

def run_backend(logging, playground_dir):
    from cetech_playground.backend import main
    main()


def run_frontend(logging, playground_dir):
    def compile_map(d, f):
        return 'src/cetech_playground/ui/', f

    # good for develop and first run.
    compileUiDir(
        dir='ui',
        recurse=True,
        map=compile_map
    )

    QDir.setCurrent(playground_dir)

    QT_LOGGER = logging.getLogger("Qt5")
    LOGGER_MAP = {
        QtCore.QtDebugMsg: QT_LOGGER.debug,
        QtCore.QtInfoMsg: QT_LOGGER.info,
        QtCore.QtWarningMsg: QT_LOGGER.warning,
        QtCore.QtCriticalMsg: QT_LOGGER.critical,
        QtCore.QtFatalMsg: QT_LOGGER.error,
    }
    qInstallMessageHandler(lambda t, c, m: LOGGER_MAP[t](m))

    from cetech_playground.frontend.main import main

    main()


def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

    with open(os.path.join(PLAYGROUND_DIR, 'logging.yaml')) as f:
        D = yaml.load(f)

        import logging.config

        logging.config.dictConfig(D)

    if not args.frontend and not args.backend:
        need_frontend = True
        need_backend = True
    else:
        need_frontend = args.frontend
        need_backend = args.backend

    if need_backend:
        backend = Process(target=run_backend, kwargs=dict(logging=logging, playground_dir=PLAYGROUND_DIR))
        backend.start()

    if need_frontend:
        frontend = Process(target=run_frontend, kwargs=dict(logging=logging, playground_dir=PLAYGROUND_DIR))
        frontend.start()
        frontend.join()

        if need_backend:
            backend.terminate()
            backend.join(timeout=1)
            return

    elif need_backend:
        backend.join(timeout=1)


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################

########################################################################################################################
