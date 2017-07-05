import PyQt5
import ctypes
import os_sdl
import platform
import pytest
import sys
from PyQt5.QtWidgets import QApplication

############
# SHITCODE #
########################################################################################################################
_platform = platform.system().lower()

ROOT_DIR = os_sdl.path.join(os_sdl.path.dirname(os_sdl.path.abspath(__file__)), os_sdl.pardir, os_sdl.pardir)
LIB_DIR = os_sdl.path.join(ROOT_DIR,
                       'externals', 'build',
                       "%s%s" % (_platform if _platform != 'darwin' else 'osx', platform.architecture()[0][0:2]),
                       'release', 'lib')

if _platform == 'windows':
    QApplication.addLibraryPath(os_sdl.path.join(os_sdl.path.dirname(PyQt5.__file__), "plugins"))
    ctypes.windll.LoadLibrary(
        os_sdl.path.join(ROOT_DIR, 'externals', 'build', 'windows64', 'Release', 'lib', 'nanomsg.dll'))
    # ctypes.windll.LoadLibrary('nanomsg.dll')
    sys.path.insert(0, 'C:\Python34\lib\site-packages')

else:
    _original_load = ctypes.cdll.LoadLibrary
    ctypes.cdll.LoadLibrary = lambda x: _original_load(os_sdl.path.join(LIB_DIR, x))

sys.path.insert(0, os_sdl.path.join(ROOT_DIR, "playground", "src"))

instance_counter = 0

from cetech import engine as engine


@pytest.fixture(scope='function')
def _build_dir(tmpdir_factory):
    instance = engine.Instance("compile test", os_sdl.path.join(ROOT_DIR, "bin"), os_sdl.path.join(ROOT_DIR, "externals/build"))

    global instance_counter

    build_dir = os_sdl.path.join(ROOT_DIR, "engine", "test", "data", "build")
    build_dir = os_sdl.path.abspath(build_dir)

    ipc_addr = os_sdl.path.join(ROOT_DIR, "engine", "test", "data", "tmp")
    ipc_addr = os_sdl.path.abspath(ipc_addr)
    if not os_sdl.path.exists(ipc_addr):
        os_sdl.makedirs(ipc_addr)

    instance_counter += 1

    instance.run_develop(
        build_dir,
        os_sdl.path.join(ROOT_DIR, "engine", "test", "data", "src"),
        core_dir=os_sdl.path.join(ROOT_DIR, "core"),
        compile_=True,
        check=True,
        daemon=True,
        rpc_addr="ipc://%s/test_compiler%s_rpc.ipc" % (ipc_addr, instance_counter),
        log_addr="ipc://%s/test_compiler%s_log.ipc" % (ipc_addr, instance_counter),
        pub_addr="ipc://%s/test_compiler%s_pub.ipc" % (ipc_addr, instance_counter),
        push_addr="ipc://%s/test_compiler%s_push.ipc" % (ipc_addr, instance_counter),
        lock=False
    )

    return build_dir


@pytest.yield_fixture(scope="function")
def engine_instance(request, _build_dir):
    instance = engine.Instance("test", os_sdl.path.join(ROOT_DIR, "bin"), os_sdl.path.join(ROOT_DIR, "externals/build"))
    global instance_counter

    ipc_addr = os_sdl.path.join(ROOT_DIR, "engine", "test", "data", "tmp")
    ipc_addr = os_sdl.path.abspath(ipc_addr)

    instance_counter += 1

    instance.run_develop(
        os_sdl.path.join(ROOT_DIR, "engine", "test", "data", "build"),
        os_sdl.path.join(ROOT_DIR, "engine", "test", "data", "src"),
        core_dir=os_sdl.path.join(ROOT_DIR, "core"),
        rpc_addr="ipc://%s/test%s_rpc.ipc" % (ipc_addr, instance_counter),
        log_addr="ipc://%s/test%s_log.ipc" % (ipc_addr, instance_counter),
        pub_addr="ipc://%s/test%s_pub.ipc" % (ipc_addr, instance_counter),
        push_addr="ipc://%s/test%s_push.ipc" % (ipc_addr, instance_counter),
        daemon=True,
        lock=True
    )

    while not instance.ready:
        pass

    api = instance.console_api

    yield api

    api.quit()
    instance.kill()
