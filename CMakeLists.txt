################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")


include_directories(engine/sources)
include_directories(celib/sources)

add_definitions(-DCETECH_STATIC_PLUGINS)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONFIGURATION debug)
    add_definitions(-DCETECH_DEBUG)
else ()
    set(CONFIGURATION release)
endif ()


if (APPLE)
    set(PLATFORM_ID darwin64)

    add_definitions(-DCETECH_COLORED_LOG)
    add_definitions(-DCETECH_DARWIN)

elseif (UNIX)
    set(PLATFORM_ID linux64)

    add_definitions(-DCETECH_LINUX)
    add_definitions(-DCETECH_COLORED_LOG)

elseif (WIN32)
    set(PLATFORM_ID windows64)

    add_definitions(-DCETECH_WINDOWS)
endif ()

link_directories(externals/build/${PLATFORM_ID}/release/lib/)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_ID})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_ID})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_ID})


################################################################################
# Sources
################################################################################

set(EXTERNAL_SOURCE_FILES
        ########################################################################
        # Externals: MPack
        ########################################################################
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-common.c
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-expect.c
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-node.c
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-platform.c
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-reader.c
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-writer.c
        celib/sources/celib/os/thread_types.h)

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        ########################################################################
        # CELIB: Containers
        ########################################################################
        celib/sources/celib/containers/tests/test_array.cc
        celib/sources/celib/containers/tests/test_map.cc

        ########################################################################
        # CELIB: Memory
        ########################################################################
        celib/sources/celib/memory/private/allocator.c
        celib/sources/celib/memory/private/memory.c
        celib/sources/celib/os/tests/test_memory.cc


        ########################################################################
        # CELIB: string
        ########################################################################
        celib/sources/celib/string/tests/test_string.cc
        celib/sources/celib/string/private/string.c


        ########################################################################
        # CELIB: Log
        #######################################################################
        celib/sources/celib/log/private/stdout_handler.c
        celib/sources/celib/log/private/nanomsg_handler.c
        celib/sources/celib/log/private/logdb_handler.c
        celib/sources/celib/log/private/log_system.c
        celib/sources/celib/log/tests/test_log.cc

        ########################################################################
        # CELIB: File
        ########################################################################
        celib/sources/celib/os/private/vio.c
        celib/sources/celib/os/private/sdl_vio.c
        celib/sources/celib/os/private/file.c
        celib/sources/celib/os/private/posix_stacktrace.c
        celib/sources/celib/os/tests/test_vfile.cc

        ########################################################################
        # CELIB: Crypto
        ########################################################################
        celib/sources/celib/crypto/private/hash.c
        celib/sources/celib/crypto/tests/test_murmur.cc

        ########################################################################
        # CELIB: Math
        ########################################################################
        celib/sources/celib/math/tests/test_vec2f.cc
        celib/sources/celib/math/tests/test_vec3f.cc
        celib/sources/celib/math/tests/test_vec4f.cc
        celib/sources/celib/math/tests/test_quatf.cc
        celib/sources/celib/math/tests/test_mat44f.cc

        ########################################################################
        # CELIB: Yaml
        ########################################################################
        celib/sources/celib/yaml/private/yaml.cc
        celib/sources/celib/yaml/tests/test_yaml.cc


        celib/sources/celib/os/private/sdl_time.c
        celib/sources/celib/os/private/sdl_object.c
        celib/sources/celib/os/private/path.c
        celib/sources/celib/os/private/sdl_thread.c
        celib/sources/celib/os/private/sdl_cpu.c
        celib/sources/celib/stringid/private/stringid.c
        celib/sources/celib/window/private/sdl_window.c
        celib/sources/celib/log/private/log_system.c

        ########################################################################
        # CETECH: Plugin system
        ########################################################################
        engine/sources/engine/core/private/plugin.c
        engine/sources/engine/core/private/plugin.c

        engine/sources/engine/luasys/private/luasys.c
        engine/sources/engine/luasys/private/api/api_keyboard.c
        engine/sources/engine/luasys/private/api/api_log.c
        engine/sources/engine/luasys/private/api/api_mouse.c
        engine/sources/engine/luasys/private/api/api_gamepad.c
        engine/sources/engine/luasys/private/api/api_application.c
        engine/sources/engine/luasys/private/api/api_resouce_compilator.c
        engine/sources/engine/luasys/private/api/api_resouce_manager.c
        engine/sources/engine/luasys/private/api/api_renderer.c
        engine/sources/engine/luasys/private/api/api_world.c
        engine/sources/engine/luasys/private/api/api_unit.c
        engine/sources/engine/luasys/private/api/api_transform.c
        engine/sources/engine/luasys/private/api/api_vec2f.c
        engine/sources/engine/luasys/private/api/api_vec3f.c
        engine/sources/engine/luasys/private/api/api_vec4f.c
        engine/sources/engine/luasys/private/api/api_quatf.c
        engine/sources/engine/luasys/private/api/api_mat44f.c
        engine/sources/engine/luasys/private/api/api_mesh.c
        engine/sources/engine/luasys/private/api/api_material.c
        engine/sources/engine/luasys/private/api/api_package.c
        engine/sources/engine/luasys/private/api/api_level.c
        engine/sources/engine/luasys/private/api/api_scenegraph.c

        engine/sources/engine/develop/private/console_server.c
        engine/sources/engine/core/private/memory.c

        engine/sources/engine/core/tests/test_filesystem.cc
        engine/sources/engine/core/private/filesystem.c
        engine/sources/engine/core/private/cvar.c
        engine/sources/engine/core/private/application.c
        engine/sources/engine/core/private/task.c
        engine/sources/engine/core/private/resource.c
        engine/sources/engine/core/private/resource_package.c
        engine/sources/engine/world/private/unit.c
        engine/sources/engine/core/private/entcom.c
        engine/sources/engine/world/private/world.c
        engine/sources/engine/world/private/level.c

        engine/sources/engine/machine/private/machine.c
        engine/sources/engine/machine/private/sdl2/sdl_mouse.c
        engine/sources/engine/machine/private/sdl2/sdl_keyboard.c
        engine/sources/engine/machine/private/sdl2/sdl_gamepad.c
        engine/sources/engine/machine/private/sdl2/sdl_machine.c

        engine/sources/engine/input/private/keyboard.c
        engine/sources/engine/input/private/mouse.c
        engine/sources/engine/input/private/gamepad.c

        engine/sources/engine/develop/private/resource_compiler.c

        engine/sources/engine/renderer/private/bgfx/renderer.c
        engine/sources/engine/renderer/private/bgfx/texture.c
        engine/sources/engine/renderer/private/bgfx/shader.c
        engine/sources/engine/renderer/private/bgfx/material.c
        engine/sources/engine/renderer/private/bgfx/scene.c
        engine/sources/engine/renderer/private/bgfx/mesh_renderer.c

        engine/sources/engine/world/private/transform.c
        engine/sources/engine/world/private/scenegraph.c
        engine/sources/engine/world/private/camera.c

        )

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES}
        engine/sources/engine/main.c)

################################################################################
# Release sources
################################################################################

if (UNIX)
    set(RELEASE_LIBS_LINUX
            jemalloc_pic.a
            pthread
            anl # NANOMSG
            dl
            X11
            GL
            z
            )
endif ()

if (WIN32)
    set(RELEASE_LIBS_WINDOWS
            zlibstatic
            )
endif ()

set(RELEASE_LIBS
        ${RELEASE_LIBS_LINUX}
        ${RELEASE_LIBS_WINDOWS}

        nanomsg.a
        bgfxRelease.a
        yaml-cpp.a

        luajit.a
        sqlite3.a
        SDL2.a
        assimp.a
        )

set(RELEASE_LIBS2
        ${RELEASE_LIBS_LINUX}
        ${RELEASE_LIBS_WINDOWS}

        nanomsg
        bgfx-shared-libRelease
        sqlite3
        assimp
        SDL2
        yaml-cpp
        luajit
        )

################################################################################
# Develop sources
################################################################################
set(DEVELOP_LIBS
        ${RELEASE_LIBS}

        )

include_directories(externals/build/${PLATFORM_ID}/release/include)

################################################################################
# Cetech RUNTIME
################################################################################
add_executable(cetech ${MAIN_SOURCE_FILES})
target_compile_definitions(cetech PUBLIC -DCETECH_RELEASE)
target_link_libraries(cetech ${RELEASE_LIBS})
target_include_directories(cetech PUBLIC externals/build/${PLATFORM_ID}/release/)
target_include_directories(cetech PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

################################################################################
# Cetech DEVELOP
################################################################################
add_executable(cetech_develop ${MAIN_SOURCE_FILES})
target_compile_definitions(cetech_develop PUBLIC -DCETECH_DEVELOP)
target_link_libraries(cetech_develop ${DEVELOP_LIBS})
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

#add_library(cetech_develop_lib SHARED ${SOURCE_FILES})
#target_link_libraries(cetech_develop_lib ${RELEASE_LIBS2})
#target_compile_definitions(cetech_develop_lib PUBLIC -DCETECH_DEVELOP)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

#set_target_properties(plugin_example PROPERTIES PREFIX "")

add_library(plugin_example SHARED engine/sources/engine/plugin_example.c)
set_target_properties(plugin_example PROPERTIES PREFIX "")

