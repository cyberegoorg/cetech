import ctypes
import os
import platform
from time import sleep

import PyQt5
import sys

import pytest
from PyQt5.QtWidgets import QApplication

############
# SHITCODE #
########################################################################################################################
_platform = platform.system().lower()

ROOT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, os.pardir)
LIB_DIR = os.path.join(ROOT_DIR,
                       'externals', 'build',
                       "%s%s" % (_platform if _platform != 'darwin' else 'osx', platform.architecture()[0][0:2]),
                       'release', 'lib')

if _platform == 'windows':
    QApplication.addLibraryPath(os.path.join(os.path.dirname(PyQt5.__file__), "plugins"))
    ctypes.windll.LoadLibrary(
        os.path.join(ROOT_DIR, 'externals', 'build', 'windows64', 'Release', 'lib', 'nanomsg.dll'))
    # ctypes.windll.LoadLibrary('nanomsg.dll')
    sys.path.insert(0, 'C:\Python34\lib\site-packages')

else:
    _original_load = ctypes.cdll.LoadLibrary
    ctypes.cdll.LoadLibrary = lambda x: _original_load(os.path.join(LIB_DIR, x))

sys.path.insert(0, os.path.join(ROOT_DIR, "playground", "src"))

instance_counter = 0


@pytest.fixture(scope="function")
def engine_instance(request, tmpdir):
    from cetech.project import EngineInstance

    instance = EngineInstance("test", os.path.join(ROOT_DIR, "build"))
    global instance_counter
    port = 4444 + (instance_counter * 2)
    instance_counter += 1

    instance.run_cetech_develop(
        tmpdir.join("data", "build"),
        os.path.join(ROOT_DIR, "data", "src"),
        core_dir=os.path.join(ROOT_DIR, "core"),
        port=port, protocol="tcp",
        compile_=True, continue_=True
    )

    sleep(0.2)

    api = instance.create_console_api()
    api.connect()

    api.wait()

    yield api

    api.quit()
    api.disconnect()
    instance.kill(True)
