#!/usr/bin/env python

""" CETech engine external lib build script
"""

###########
# IMPORTS #
########################################################################################################################

import argparse
import multiprocessing
import os
import platform
import shutil
import subprocess
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

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.path.pardir))
EXTERNAL_DIR = os.path.join(ROOT_DIR, 'externals')
EXTERNAL_BUILD_DIR = os.path.join(EXTERNAL_DIR, 'build')
EXTERNAL_SRC_DIR = os.path.join(EXTERNAL_DIR, 'src')
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
    'darwin64',
    'windows64'
}

# EXTERNALS
with open(os.path.join(EXTERNAL_DIR, 'externals.yml')) as f:
    EXTERNALS = yaml.load(f.read())

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
    "-c", "--config",
    help='Build configuration',
    default='debug', choices=CONFIG)

ARGS_PARSER.add_argument(
    "-e", "--external",
    help='Build configuration',
    default='', choices=[''] + [x for x in EXTERNALS.keys()])

ARGS_PARSER.add_argument(
    "-p", "--platform",
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
def clone(name, body, verbose):
    clone_dir = os.path.join(EXTERNAL_SRC_DIR, name)

    print('Clone: %s' % name)

    clone_cmds = [body['clone']] if isinstance(body['clone'], str) else body['clone']

    for clone_cmd in clone_cmds:
        cmd = clone_cmd.split(' ')

        if cmd[0] == 'git':
            if os.path.exists(clone_dir):
                continue

            cmd = cmd + [clone_dir]
            _stdout = subprocess.DEVNULL if not verbose else None
            subprocess.check_call(cmd, stdout=_stdout, stderr=subprocess.STDOUT, universal_newlines=True)

        elif cmd[0] == 'download':
            print('Download: %s' % cmd[1])

            if not os.path.exists(clone_dir):
                os.mkdir(clone_dir)

            urllib.request.urlretrieve(cmd[1], os.path.abspath(os.path.join(clone_dir, cmd[2])))


########################################################################################################################
# Build
########################################################################################################################
def build(name, body, platform_, job_count_str, verbose):
    clone_dir = os.path.join(EXTERNAL_SRC_DIR, name)

    print('Build: %s' % name)

    commands = body['build'][platform_]
    commands = [commands] if isinstance(commands, str) else commands

    os.chdir(clone_dir)

    for cmd in commands:
        cmd_lst = cmd.split(' ')

        if cmd_lst[0] == 'make':
            cmd_lst.insert(1, '-j')
            cmd_lst.insert(2, job_count_str)

        elif cmd_lst[0] == 'cmake':
            if "windows" in platform_:
                cmd_lst.append('-G')
                cmd_lst.append('Visual Studio 14 2015 Win64')

            cmd_lst.append('-DCMAKE_INCLUDE_PATH=%s' % os.path.join(EXTERNAL_BUILD_DIR, platform_, 'include'))
            cmd_lst.append(clone_dir)

        elif cmd_lst[0] == 'devenv':
            cmd_lst[0] = os.path.join('C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE', 'devenv')

        elif cmd_lst[0] == 'extract':
            cmd_lst[0] = os.path.join(EXTERNAL_DIR, 'tools', 'windows64', '7zip', '7za.exe')
            cmd_lst.insert(1, 'e')
            cmd_lst.insert(2, '-aoa')

        if verbose:
            print("Build cmd: %s" % ' '.join(cmd_lst))

        _stdout = subprocess.DEVNULL if not verbose else None
        subprocess.check_call(cmd_lst, stdout=_stdout, stderr=subprocess.STDOUT, universal_newlines=True)

    os.chdir(ROOT_DIR)


########################################################################################################################
# Install
########################################################################################################################
def copytree(src, dst, symlinks=False, ignore=None, copy_function=shutil.copy2,
             ignore_dangling_symlinks=False):
    """Recursively copy a directory tree.

    The destination directory must not already exist.
    If exception(s) occur, an Error is raised with a list of reasons.

    If the optional symlinks flag is true, symbolic links in the
    source tree result in symbolic links in the destination tree; if
    it is false, the contents of the files pointed to by symbolic
    links are copied. If the file pointed by the symlink doesn't
    exist, an exception will be added in the list of errors raised in
    an Error exception at the end of the copy process.

    You can set the optional ignore_dangling_symlinks flag to true if you
    want to silence this exception. Notice that this has no effect on
    platforms that don't support os.symlink.

    The optional ignore argument is a callable. If given, it
    is called with the `src` parameter, which is the directory
    being visited by copytree(), and `names` which is the list of
    `src` contents, as returned by os.listdir():

        callable(src, names) -> ignored_names

    Since copytree() is called recursively, the callable will be
    called once for each directory that is copied. It returns a
    list of names relative to the `src` directory that should
    not be copied.

    The optional copy_function argument is a callable that will be used
    to copy each file. It will be called with the source path and the
    destination path as arguments. By default, copy2() is used, but any
    function that supports the same signature (like copy()) can be used.

    """
    names = os.listdir(src)
    if ignore is not None:
        ignored_names = ignore(src, names)
    else:
        ignored_names = set()

    os.makedirs(dst, exist_ok=True)
    errors = []
    for name in names:
        if name in ignored_names:
            continue
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if os.path.islink(srcname):
                linkto = os.readlink(srcname)
                if symlinks:
                    # We can't just leave it to `copy_function` because legacy
                    # code with a custom `copy_function` may rely on copytree
                    # doing the right thing.
                    os.symlink(linkto, dstname)
                    shutil.copystat(srcname, dstname, follow_symlinks=not symlinks)
                else:
                    # ignore dangling symlink if the flag is on
                    if not os.path.exists(linkto) and ignore_dangling_symlinks:
                        continue
                    # otherwise let the copy occurs. copy2 will raise an error
                    if os.path.isdir(srcname):
                        copytree(srcname, dstname, symlinks, ignore,
                                 copy_function)
                    else:
                        copy_function(srcname, dstname)
            elif os.path.isdir(srcname):
                copytree(srcname, dstname, symlinks, ignore, copy_function)
            else:
                # Will raise a SpecialFileError for unsupported file types
                copy_function(srcname, dstname)
        # catch the Error from the recursive copytree so that we can
        # continue with other files
        except shutil.Error as err:
            errors.extend(err.args[0])
        except OSError as why:
            errors.append((srcname, dstname, str(why)))
    try:
        shutil.copystat(src, dst)
    except OSError as why:
        # Copying file access times may fail on Windows
        if getattr(why, 'winerror', None) is None:
            errors.append((src, dst, str(why)))
    if errors:
        raise shutil.Error(errors)
    return dst


def install(name, body, platform_):
    clone_dir = os.path.join(EXTERNAL_SRC_DIR, name)

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

            elif name == 'copyfile':
                try:
                    os.makedirs(os.path.dirname(dst_dir))
                except FileExistsError:
                    pass

                shutil.copy(src, dst_dir)

            elif name == 'copytree':
                try:
                    copytree(src, dst_dir)
                except FileExistsError:
                    pass

    os.chdir(ROOT_DIR)


########################################################################################################################
# Job
########################################################################################################################
def do_job(name, body, job_list, config, platform_, external, only_clone, job_count, verbose):
    suported_platform = body.get('platforms', PLATFORMS)
    if isinstance(suported_platform, str):
        suported_platform = [suported_platform]

    if platform_ not in suported_platform:
        return

    clone(name, body, verbose)
    if only_clone:
        return

    if 'depend' in body:
        depend = body['depend']
        print(depend)
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
    shutil.rmtree(EXTERNAL_BUILD_DIR, ignore_errors=True)
    shutil.rmtree(EXTERNAL_SRC_DIR, ignore_errors=True)


def main(args=None):
    """ ENTRY POINT
    """

    args_parse = ARGS_PARSER.parse_args(args=args)

    print('Platform: %s' % args_parse.platform)

    action = args_parse.action
    if action == '':
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
