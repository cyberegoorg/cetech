#!/usr/bin/env python

""" CETech engine external lib build script
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
import urllib.request
import yaml

###########
# GLOBALS #
########################################################################################################################

CPU_COUNT = multiprocessing.cpu_count()
CPU_COUNT_STR = str(CPU_COUNT)

OS_NAME = platform.system().lower()
OS_ARCH = 64 if sys.maxsize > 2 ** 32 else 32

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__))))
EXTERNAL_DIR = os.path.join(ROOT_DIR, 'external')
EXTERNAL_BUILD_DIR = os.path.join(EXTERNAL_DIR, '.build')

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
    'linux64',
    'darwin64'
}


# EXTERNALS
with open(os.path.join(ROOT_DIR, 'externals.yml')) as f:
    EXTERNALS = yaml.load(f.read())

EXTERNALS.update({'': None})

########
# ARGS #
########################################################################################################################

ARGS_PARSER = argparse.ArgumentParser(description='CETech external dependencies build script')

ARGS_PARSER.add_argument(
        "action",
        help="Build action",
        nargs='?', type=str, default='', choices=ACTIONS)

ARGS_PARSER.add_argument(
        "--clone",
        help='Only clone externals',
        action='store_true')

ARGS_PARSER.add_argument(
        "--config",
        help='Build configuration',
        default='debug', choices=CONFIG)

ARGS_PARSER.add_argument(
        "--external",
        help='Build configuration',
        default='', choices=[x for x in EXTERNALS.keys()])

ARGS_PARSER.add_argument(
        "--platform",
        help='Target platform',
        default=DEFAULT_BUILD, choices=PLATFORMS)

###########
# PROGRAM #
########################################################################################################################

def make_externals(config, platform, external='', only_clone=False):
    """Make externals build

    :param config: Build configuration.
    :param platform: Build platform.
    :param external: External.
    :param only_clone: Do not run build, only create projects files.
    """

    try:
        os.mkdir(EXTERNAL_DIR)
    except FileExistsError:
        pass

    externals = {external: EXTERNALS[external]} if external != '' else EXTERNALS

    # CLONING
    for k, v in externals.items():
        if k == '':
            continue

        clone_dir = os.path.join('external', k)

        clone_cmds = [v['clone']] if isinstance(v['clone'], str) else v['clone']

        for clone in clone_cmds:
            cmd = clone.split(' ') + [clone_dir]
            if cmd[0] == 'git':
                if os.path.exists(clone_dir):
                    print('external %s exist. skiping...' % k)
                    continue

                print('Cloning %s' % k)
                subprocess.check_call(cmd)

            elif cmd[0] == 'download':
                urllib.request.urlretrieve(cmd[1], os.path.abspath(os.path.join(clone_dir, cmd[2])))
                pass

    if only_clone:
        return

    # BUILD
    for k, v in externals.items():
        if k == '':
            continue

        clone_dir = os.path.join('external', k)

        if 'build' not in v:
            continue

        print('Building %s' % k)

        commands = v['build'][platform]
        os.chdir(clone_dir)

        for cmd in commands:
            cmd_lst = cmd.split(' ')

            if cmd_lst[0] == 'make':
                cmd_lst.insert(1, '-j')
                cmd_lst.insert(2, CPU_COUNT_STR)

            print(cmd_lst)

            subprocess.check_call(cmd_lst, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT,universal_newlines=True)

        os.chdir(ROOT_DIR)

    # INSTALL
    for k, v in externals.items():
        if k == '':
            continue

        clone_dir = os.path.join('external', k)

        if 'install' not in v:
            continue

        print('Instaling %s' % k)

        commands = v['install'][platform]
        os.chdir(clone_dir)

        for cmd in commands:
            for name, params in cmd.items():
                src, dst_dir = params
                dst_dir = os.path.join(EXTERNAL_BUILD_DIR, platform, dst_dir)

                if name == 'copy':
                    try:
                        os.makedirs(dst_dir)
                    except FileExistsError:
                        pass

                    shutil.copy(src, dst_dir)

                if name == 'copytree':
                    try:
                        shutil.copytree(src, dst_dir)
                    except FileExistsError as e:
                        pass

        os.chdir(ROOT_DIR)


def clean():
    """ Remove build dir.
    """

    print('Cleaning...')
    shutil.rmtree(EXTERNAL_DIR, ignore_errors=True)


def main(args=None):
    """ ENTRY POINT
    """

    args = ARGS_PARSER.parse_args(args=args)

    action = args.action
    if action == '':
        make_externals(config=args.config, platform=args.platform, external=args.external, only_clone=args.clone)

    elif action == 'clean':
        clean()


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
