#!/usr/bin/env python

""" CETech engine build script
"""

###########
# IMPORTS #
########################################################################################################################

import os
import subprocess
import argparse
import platform
import shutil
import multiprocessing
import sys

###########
# GLOBALS #
########################################################################################################################

CPU_COUNT = multiprocessing.cpu_count()
CPU_COUNT_STR = str(CPU_COUNT)

OS_NAME = platform.system().lower()
OS_ARCH = 64 if sys.maxsize > 2 ** 32 else 32

ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir)
BUILD_DIR = os.path.abspath(os.path.join(ROOT_DIR, 'build'))
BIN_DIR = os.path.abspath(os.path.join(ROOT_DIR, 'bin'))
EXTERNAL_BUILD_DIR = os.path.abspath(os.path.join(ROOT_DIR, 'externals', 'build'))

DEFAULT_BUILD = "%s%s" % (OS_NAME, OS_ARCH)

##########
# CONFIG #
########################################################################################################################

# Command line actions.
ACTIONS = {
    '',
    'build',
    'clean'
}

BUILD_ACTION = ('build', '')

# Build platform.
PLATFORMS = {
    'linux64',
}


def make_make(config, platform_, debug):
    if not debug:
        cmds = ['make']
    else:
        cmds = ['make']

    return cmds


PLATFORMS_MAKE = {
    'linux64': make_make
}

########
# ARGS #
########################################################################################################################

ARGS_PARSER = argparse.ArgumentParser(description='CETech build script')

ARGS_PARSER.add_argument(
    "action",
    help="Build action",
    nargs='?', type=str, default='', choices=ACTIONS)

ARGS_PARSER.add_argument(
    "-g", "--generate",
    help='Only generate project files',
    action='store_true')

# ARGS_PARSER.add_argument(
#     "-c", "--config",
#     help='Build configuration',
#     default='develop', choices=CONFIG)

ARGS_PARSER.add_argument(
    "-d", "--debug",
    help='Debug build',
    action='store_true')

ARGS_PARSER.add_argument(
    "-p", "--platform",
    default=DEFAULT_BUILD, choices=PLATFORMS, help='Target platform')


###########
# PROGRAM #
########################################################################################################################

def run_cmake(config, platform_, action=''):
    """Run platform specific genie command.
    """

    print('Runing cmake')

    os.makedirs(BUILD_DIR, exist_ok=True)
    os.chdir(BUILD_DIR)

    cmds = ['cmake', os.pardir, '-DCMAKE_C_COMPILER=clang', '-DCMAKE_CXX_COMPILER=clang++', '-DCMAKE_BUILD_TYPE=Debug']

    subprocess.check_call(cmds)


def make(config, platform_, debug, generate_only=False):
    """Make build

    :param config: Build configuration.
    :param platform_: Build platform.
    :param generate_only: Do not run build, only create projects files.
    """
    run_cmake(config=config, platform_=platform_)

    if not generate_only:
        cmds = PLATFORMS_MAKE[platform_](config=config, platform_=platform_, debug=debug)
        subprocess.check_call(cmds)


def clean(config, platform_):
    """ Remove build dir.
    """

    print('Cleaning...')

    try:
        shutil.rmtree(BUILD_DIR)
    except FileNotFoundError:
        pass

    try:
        shutil.rmtree(BIN_DIR)
    except FileNotFoundError:
        pass

def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

    action = args.action
    if action in BUILD_ACTION:
        make(config=None,  # args.config,
             platform_=args.platform,
             generate_only=args.generate,
             debug=args.debug)

    elif action == 'clean':
        clean(config=None, platform_=args.platform)


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
