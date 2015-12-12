#!/usr/bin/env python

###########
# IMPORTS #
########################################################################################################################
import ctypes
import os
import sys
import platform

from PyQt5.QtCore import QDir
from PyQt5.QtGui import QIcon
from PyQt5.uic import compileUiDir

###########
# GLOBALS #
########################################################################################################################
ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
PLAYGROUND_DIR = os.path.join(ROOT_DIR, 'playground')

_platform = platform.system().lower()
LIB_DIR = os.path.join(ROOT_DIR,
                       'externals',
                       'build',
                       "%s%s" % (_platform if _platform != 'darwin' else 'osx', platform.architecture()[0][0:2]),
                       'dynlib')
_original_load = ctypes.cdll.LoadLibrary
ctypes.cdll.LoadLibrary = lambda x: _original_load(os.path.join(LIB_DIR, x))

sys.path.insert(0, os.path.join(PLAYGROUND_DIR, 'src'))
QDir.setCurrent(PLAYGROUND_DIR)

QIcon.setThemeSearchPaths([os.path.join(PLAYGROUND_DIR, 'icons', 'breeze')])
QIcon.setThemeName('Breeze Dark')


def compile_map(d, f):
    return 'src/playground/ui/', f


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    # good for develop and first run.
    compileUiDir(
            dir='ui',
            recurse=True,
            map=compile_map
    )

    from playground.main import main

    main()
########################################################################################################################
