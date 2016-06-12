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

# Command line actions.
ACTIONS = {
    '',
    'build',
    'clean'
}

BUILD_ACTION = ('build', '')

# Build config.
CONFIG = {
    'develop',
    'runtime'
}

# Build platform.
PLATFORMS = {
    'windows64',
    'linux64',
    'darwin64',
}

# Build platform.
PLATFORMS_PROTOBUILD = {
    'windows64': 'Windows',
    'linux64': 'Linux',
    'darwin64': 'MacOS'
}

# Build platform.
PLATFORMS_SLN = {
    'windows64': 'CETech.Windows.sln',
    'linux64': 'CETech.Linux.sln',
    'darwin64': 'CETech.MacOS.sln'
}

# Build platform.
PROTOBUILD_CONFIG = {
    'runtime': ['-disable', 'Develop']
}

def make_vs(config, platform_, debug):
    cmds = [os.path.join('C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE', 'devenv'),
            PLATFORMS_SLN[platform_], '/build']

    if not debug:
        cmds.append('Release')
    else:
        cmds.append('Debug')

    return cmds


def make_xbuild(config, platform_, debug):
    cmds = ['xbuild', PLATFORMS_SLN[platform_]]

    if not debug:
        cmds.append('/p:configuration=Release')
    else:
        cmds.append('/p:configuration=Debug')

    return cmds


PLATFORMS_MAKE = {
    'windows64': make_vs,
    'linux64': make_xbuild
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

def run_protobuild(config, platform_, action=''):
    """Run platform specific genie command.
    """
    print('Runing Protobuild.exe.')

    if 'windows' in platform_:
        cmds = [os.path.join(ROOT_DIR, 'Protobuild.exe')]
    else:
        cmds = ['mono', os.path.join(ROOT_DIR, 'Protobuild.exe')]

    # if config in PROTOBUILD_CONFIG:
    #     cmds.extend(PROTOBUILD_CONFIG[config])

    if action in BUILD_ACTION:
        cmds.append('--generate')
        cmds.append(PLATFORMS_PROTOBUILD[platform_])
    elif action == 'clean':
        cmds.append('--clean')

    subprocess.check_call(cmds)

def make(config, platform_, debug, generate_only=False):
    """Make build

    :param config: Build configuration.
    :param platform_: Build platform.
    :param generate_only: Do not run build, only create projects files.
    """
    run_protobuild(config=config, platform_=platform_)

    if not generate_only:
        cmds = PLATFORMS_MAKE[platform_](config=config, platform_=platform_, debug=debug)
        subprocess.check_call(cmds)


def clean(config, platform_):
    """ Remove build dir.
    """

    print('Cleaning...')
    shutil.rmtree(os.path.join('sources', 'CETech', 'bin'))
    shutil.rmtree(os.path.join('sources', 'CETech', 'obj'))
    run_protobuild(config=config, platform_=platform_, action='clean')


def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

    action = args.action
    if action == '':
        make(config=None, #args.config,
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
