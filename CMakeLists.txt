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
add_definitions(-DCELIB_USE_SDL)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONFIGURATION debug)
    add_definitions(-DCETECH_DEBUG)
    add_definitions(-DCELIB_DEBUG)
    add_definitions(-DCELIB_DEBUG)
else ()
    set(CONFIGURATION release)
endif ()


if (APPLE)
    set(PLATFORM_ID darwin64)

    add_definitions(-DCELIB_COLORED_LOG)
    add_definitions(-DCETECH_DARWIN)
    add_definitions(-DCELIB_DARWIN)

elseif (UNIX)
    set(PLATFORM_ID linux64)

    add_definitions(-DCETECH_LINUX)
    add_definitions(-DCELIB_LINUX)
    add_definitions(-DCELIB_COLORED_LOG)

elseif (WIN32)
    set(PLATFORM_ID windows64)

    add_definitions(-DCETECH_WINDOWS)
    add_definitions(-DCELIB_WINDOWS)
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
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-writer.c)

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        ########################################################################
        # CELIB: Memory
        ########################################################################
        celib/sources/celib/memory/private/allocator.c
        celib/sources/celib/memory/private/allocator_malloc.c
        celib/sources/celib/memory/private/allocator_scratch.c
        celib/sources/celib/memory/private/memsys.c


        ########################################################################
        # CELIB: Log
        #######################################################################
        celib/sources/celib/log/private/stdout_handler.c
        celib/sources/celib/log/private/nanomsg_handler.c
        celib/sources/celib/log/private/logdb_handler.c
        celib/sources/celib/log/private/log_system.c

        ########################################################################
        # CELIB: File
        ########################################################################
        celib/sources/celib/filesystem/private/sdl_vio.c

        ########################################################################
        # CELIB: Crypto
        ########################################################################

        ########################################################################
        # CELIB: Math
        ########################################################################

        ########################################################################
        # CELIB: Yaml
        ########################################################################
        celib/sources/celib/yaml/private/yaml.cc

        celib/sources/celib/log/private/log_system.c
        engine/sources/engine/config/private/cvar.c

        ########################################################################
        # CETECH: Plugin system
        ########################################################################
        engine/sources/engine/plugin/private/plugin.c

        engine/sources/engine/luasys/private/luasys.c
        engine/sources/engine/luasys/private/api/api_keyboard.c
        engine/sources/engine/luasys/private/api/api_log.c
        engine/sources/engine/luasys/private/api/api_mouse.c
        engine/sources/engine/luasys/private/api/api_gamepad.c
        engine/sources/engine/luasys/private/api/api_application.c
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
        engine/sources/engine/develop/private/develop_system.c

        engine/sources/engine/resource/private/filesystem.c
        engine/sources/engine/application/private/application.c
        engine/sources/engine/task/private/task.c

        engine/sources/engine/resource/private/resource.c
        engine/sources/engine/resource/private/resource_package.c
        engine/sources/engine/resource/private/resource_compiler.c
        engine/sources/engine/world/private/unit.c
        engine/sources/engine/entcom/private/entcom.c
        engine/sources/engine/world/private/world.c
        engine/sources/engine/world/private/level.c

        celib/sources/celib/machine/private/machine.c
        celib/sources/celib/machine/private/sdl2/sdl_mouse.c
        celib/sources/celib/machine/private/sdl2/sdl_keyboard.c
        celib/sources/celib/machine/private/sdl2/sdl_gamepad.c
        celib/sources/celib/machine/private/sdl2/sdl_machine.c

        engine/sources/engine/input/private/keyboard.c
        engine/sources/engine/input/private/mouse.c
        engine/sources/engine/input/private/gamepad.c


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

add_library(plugin_example SHARED engine/sources/plugin_example/plugin_example.c)
set_target_properties(plugin_example PROPERTIES PREFIX "")

