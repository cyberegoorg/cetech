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

ARGS_PARSER.add_argument(
        "-j", "--job-count",
        help='Max job count',
        default=CPU_COUNT)

ARGS_PARSER.add_argument(
        "-v", "--verbose",
        help='Verbose mode',
        action='store_true')


###########
# PROGRAM #
########################################################################################################################

########################################################################################################################
# Clone
########################################################################################################################
def clone(name, body):
    clone_dir = os.path.join('external', name)

    clone_cmds = [body['clone']] if isinstance(body['clone'], str) else body['clone']

    for clone_cmd in clone_cmds:
        cmd = clone_cmd.split(' ')
        if cmd[0] == 'git':
            if os.path.exists(clone_dir):
                continue

            print('Clone: %s' % name)

            cmd = cmd + [clone_dir]
            subprocess.check_call(cmd)

        elif cmd[0] == 'download':
            urllib.request.urlretrieve(cmd[1], os.path.abspath(os.path.join(clone_dir, cmd[2])))
            pass


########################################################################################################################
# Build
########################################################################################################################
def build(name, body, platform_, job_count_str, verbose):
    clone_dir = os.path.join('external', name)

    print('Build: %s' % name)

    commands = body['build'][platform_]
    os.chdir(clone_dir)

    for cmd in commands:
        cmd_lst = cmd.split(' ')

        if cmd_lst[0] == 'make':
            cmd_lst.insert(1, '-j')
            cmd_lst.insert(2, job_count_str)

        elif cmd_lst[0] == 'cmake':
            cmd_lst.insert(1, '-DCMAKE_INCLUDE_PATH=%s' % os.path.join(EXTERNAL_BUILD_DIR, platform_, 'include'))
            cmd_lst.insert(2, job_count_str)

        if verbose:
            print("Build cmd: %s" % cmd_lst)

        _stdout = subprocess.DEVNULL if not verbose else None
        subprocess.check_call(cmd_lst, stdout=_stdout, stderr=subprocess.STDOUT, universal_newlines=True)

    os.chdir(ROOT_DIR)


########################################################################################################################
# Install
########################################################################################################################
def install(name, body, platform_):
    clone_dir = os.path.join('external', name)

    print('Install: %s' % name)

    commands = body['install'][platform_]
    os.chdir(clone_dir)

    for cmd in commands:
        for name, params in cmd.items():
            src, dst_dir = params
            dst_dir = os.path.join(EXTERNAL_BUILD_DIR, platform_, dst_dir)

            if name == 'copy':
                try:
                    os.makedirs(dst_dir)
                except FileExistsError:
                    pass

                shutil.copy(src, dst_dir)

            if name == 'copytree':
                try:
                    shutil.copytree(src, dst_dir)
                except FileExistsError:
                    pass

    os.chdir(ROOT_DIR)


########################################################################################################################
# Job
########################################################################################################################
def do_job(name, body, job_list, config, platform_, external, only_clone, job_count, verbose):
    clone(name, body)

    if only_clone:
        return

    if 'depend' in body:
        depend = body['depend']

        if depend in job_list:
            do_job(name=depend,
                   body=EXTERNALS[depend],
                   job_list=job_list,
                   config=config,
                   platform_=platform_,
                   external=external,
                   only_clone=only_clone,
                   job_count=job_count,
                   verbose=verbose)
            job_list.remove(depend)

    if 'build' in body:
        build(name=name,
              body=body,
              platform_=platform_,
              job_count_str=str(job_count),
              verbose=verbose)

    if 'install' in body:
        install(name=name,
                body=body,
                platform_=platform_)


def make_externals(config, platform_, external='', only_clone=False, job_count=CPU_COUNT, verbose=False):
    """Make externals build

    :type verbose: bool
    :type job_count: int
    :type only_clone: bool
    :type external: str
    :type platform_: str
    :type config: str
    """

    job_count_str = str(job_count)

    try:
        os.mkdir(EXTERNAL_DIR)
    except FileExistsError:
        pass

    externals = {external: EXTERNALS[external]} if external != '' else EXTERNALS

    job_list = {x for x in externals}

    while len(job_list):
        job_name = job_list.pop()
        job_body = externals[job_name]

        do_job(name=job_name,
               body=job_body,
               job_list=job_list,
               config=config,
               platform_=platform_,
               external=external,
               only_clone=only_clone,
               job_count=job_count,
               verbose=verbose)


def clean():
    """ Remove build dir.
    """

    print('Cleaning...')
    shutil.rmtree(EXTERNAL_DIR, ignore_errors=True)


def main(args=None):
    """ ENTRY POINT
    """

    args_parse = ARGS_PARSER.parse_args(args=args)

    action = args_parse.action
    if action == '':
        EXTERNALS.pop('')
        make_externals(config=args_parse.config,
                       platform_=args_parse.platform,
                       external=args_parse.external,
                       only_clone=args_parse.clone,
                       job_count=args_parse.job_count,
                       verbose=args_parse.verbose)

    elif action == 'clean':
        clean()


########
# MAIN #
########################################################################################################################

if __name__ == '__main__':
    main()

########################################################################################################################
