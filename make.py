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
BUILD_DIR = os.path.abspath(os.path.join(ROOT_DIR, 'build'))
EXTERNAL_BUILD_DIR = os.path.abspath(os.path.join(ROOT_DIR, 'externals', 'build'))

GENIE = os.path.join(EXTERNAL_BUILD_DIR,
                     "%s%s" % (OS_NAME, OS_ARCH),
                     'bin',
                     'genie')

DEFAULT_BUILD = "%s%s" % (OS_NAME, OS_ARCH)

##########
# CONFIG #
########################################################################################################################
LINUX_GENIE = [GENIE, '--gcc=linux-clang', 'gmake']
LINUX_BUILD = ['make', '-R', '-C', 'build/projects/gmake-linux-clang']

DARWIN_GENIE = [GENIE, '--gcc=osx', 'gmake']
DARWIN_BUILD = ['make', '-j', CPU_COUNT_STR, '-R', '-C', 'build/projects/gmake-osx']

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

ARGS_PARSER.add_argument(
        "--cc",
        help='CC')

ARGS_PARSER.add_argument(
        "--cxx",
        help='CXX')

ARGS_PARSER.add_argument(
        "-j", "--job-count",
        help='Max job count',
        default=CPU_COUNT)


###########
# PROGRAM #
########################################################################################################################

def run_genie(platform_, cc=None, cxx=None):
    """Run platform specific genie command.
    """

    cmd = PLATFORMS_GENIE[platform_]

    subprocess.check_call(cmd)


def make(config, platform_, only_genie=False, cc=None, cxx=None, job_count=CPU_COUNT):
    """Make build

    :param config: Build configuration.
    :param platform_: Build platform.
    :param only_genie: Do not run build, only create projects files.
    """
    print('Runing genie.')
    run_genie(platform_=platform_)

    if not only_genie:
        print('Building.')

        cmd = PLATFORMS_BUILD[platform_][config]

        if cmd[0] == 'make':
            cmd.insert(1, '-j')
            cmd.insert(2, CPU_COUNT_STR)

        if cc is not None:
            cmd.append('CC=%s' % cc)

        if cxx is not None:
            cmd.append('CXX=%s' % cxx)

        subprocess.check_call(cmd)


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
        make(args.config,
             args.platform,
             args.generate,
             args.cc,
             args.cxx,
             args.job_count)

    elif action == 'clean':
        clean()


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
