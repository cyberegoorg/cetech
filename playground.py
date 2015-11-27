#!/usr/bin/env python

###########
# IMPORTS #
########################################################################################################################

import os
import sys

from PyQt5.QtCore import QDir
from PyQt5.QtGui import QIcon
from PyQt5.uic import compileUiDir

###########
# GLOBALS #
########################################################################################################################

PLAYGROUND_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'playground')
sys.path.insert(0, os.path.join(PLAYGROUND_DIR, 'src'))
QDir.setCurrent(PLAYGROUND_DIR)

def compile_map(d, f):
    return 'src/playground/ui/', f

QIcon.setThemeSearchPaths([os.path.join(PLAYGROUND_DIR, 'icons')])
QIcon.setThemeName('Faenza')

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
