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

ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)))
BUILD_DIR = os.path.abspath(os.path.join(ROOT_DIR, '.build'))
EXTERNAL_BUILD_DIR = os.path.abspath(os.path.join(ROOT_DIR, 'external', '.build'))

GENIE = os.path.join(EXTERNAL_BUILD_DIR,
                     "%s%s" % (OS_NAME, OS_ARCH),
                     'bin',
                     'genie')

LINUX_GENIE = [GENIE, '--gcc=linux-clang', 'gmake']
LINUX_BUILD = ['make', '-j', CPU_COUNT_STR, '-R', '-C', '.build/projects/gmake-linux-clang']

DARWIN_GENIE = [GENIE, '--gcc=osx', 'gmake']
DARWIN_BUILD = ['make', '-j', CPU_COUNT_STR, '-R', '-C', '.build/projects/gmake-osx']

DEFAULT_BUILD = "%s%s" % (OS_NAME, OS_ARCH)

##########
# CONFIG #
########################################################################################################################

# Command line actions.
ACTIONS = {
    '',
    'clean'
}

# Build config.
CONFIG = {
    'develop',
    'debug',
    'release'
}

# Build platform.
PLATFORMS = {
    'linux64'
    'darwin64'
}

# Platform specific genie command.
PLATFORMS_GENIE = {
    'linux64': LINUX_GENIE,
    'linux32': LINUX_GENIE,
    'darwin64': DARWIN_GENIE,
}

# PLatform specific build command.
PLATFORMS_BUILD = {
    'linux64': {
        'develop': LINUX_BUILD + ['config=develop64'],
        'debug': LINUX_BUILD + ['config=debug64'],
        'release': LINUX_BUILD + ['config=release64'],
    },

    'linux32': {
        'develop': LINUX_BUILD + ['config=develop32'],
        'debug': LINUX_BUILD + ['config=debug32'],
        'release': LINUX_BUILD + ['config=release32'],
    },

    'darwin64': {
        'develop': DARWIN_BUILD + ['config=develop64'],
        'debug': DARWIN_BUILD + ['config=debug64'],
        'release': DARWIN_BUILD + ['config=release64'],
    },
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
    "--generate",
    help='Only generate project files to build dir',
    action='store_true')

ARGS_PARSER.add_argument(
    "--config",
    help='Build configuration',
    default='debug', choices=CONFIG)

ARGS_PARSER.add_argument(
    "--platform",
    default=DEFAULT_BUILD, choices=PLATFORMS, help='Target platform')


###########
# PROGRAM #
########################################################################################################################

def run_genie(platform):
    """Run platform specific genie command.
    """
    subprocess.check_call(PLATFORMS_GENIE[platform])


def make(config, platform, only_genie=False):
    """Make build

    :param config: Build configuration.
    :param platform: Build platform.
    :param only_genie: Do not run build, only create projects files.
    """
    print('Runing genie.')
    run_genie(platform=platform)

    print('Building.')
    if not only_genie:
        subprocess.check_call(PLATFORMS_BUILD[platform][config])


def clean():
    """ Remove build dir.
    """

    print('Cleaning...')
    shutil.rmtree(BUILD_DIR, ignore_errors=True)


def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

    action = args.action
    if action == '':
        make(args.config, args.platform, args.generate)

    elif action == 'clean':
        clean()


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
