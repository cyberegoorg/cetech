################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")


include_directories(engine/sources)
include_directories(engine/include)

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
        # CELIB
        ########################################################################
        engine/sources/cetech/memory/allocator.c
        engine/sources/cetech/memory/allocator_malloc.c
        engine/sources/cetech/memory/allocator_scratch.c
        engine/sources/cetech/log/log_stdout_handler.c
        engine/sources/cetech/log/log_nanomsg_handler.c
        engine/sources/cetech/log/log_db_handler.c
        engine/sources/cetech/log/log_system.c
        engine/sources/cetech/filesystem/vio_sdl.c
        engine/sources/cetech/yaml/yaml.cc
        engine/sources/cetech/log/log_system.c
        engine/sources/cetech/os/cmd_line.c
        engine/sources/cetech/filesystem/fs.c
        engine/sources/cetech/handler/handler.c
        engine/sources/cetech/sdl2/sdl2_object.h
        engine/sources/cetech/os/path.c
        engine/sources/cetech/os/process.c
        engine/sources/cetech/os/stacktrace.c
        engine/sources/cetech/string/string.c
        engine/sources/cetech/string/stringid.c
        engine/sources/cetech/sdl2/sdl_time.h
        engine/sources/cetech/filesystem/vio.c

        ########################################################################
        # CETECH
        ########################################################################
        engine/sources/cetech/module/module.c
        engine/sources/cetech/config/config.c
        engine/sources/cetech/memory/memory.c

        engine/sources/cetech/luasys/luasys.c
        engine/sources/cetech/luasys/api/api_plugin.c
        engine/sources/cetech/luasys/api/api_keyboard.c
        engine/sources/cetech/luasys/api/api_log.c
        engine/sources/cetech/luasys/api/api_mouse.c
        engine/sources/cetech/luasys/api/api_gamepad.c
        engine/sources/cetech/luasys/api/api_application.c
        engine/sources/cetech/luasys/api/api_resouce_manager.c
        engine/sources/cetech/luasys/api/api_renderer.c
        engine/sources/cetech/luasys/api/api_world.c
        engine/sources/cetech/luasys/api/api_entity.c
        engine/sources/cetech/luasys/api/api_component.c
        engine/sources/cetech/luasys/api/api_transform.c
        engine/sources/cetech/luasys/api/api_vec2f.c
        engine/sources/cetech/luasys/api/api_vec3f.c
        engine/sources/cetech/luasys/api/api_vec4f.c
        engine/sources/cetech/luasys/api/api_quatf.c
        engine/sources/cetech/luasys/api/api_mat44f.c
        engine/sources/cetech/luasys/api/api_mesh.c
        engine/sources/cetech/luasys/api/api_material.c
        engine/sources/cetech/luasys/api/api_package.c
        engine/sources/cetech/luasys/api/api_level.c
        engine/sources/cetech/luasys/api/api_scenegraph.c

        engine/sources/cetech/develop/console_server.c
        engine/sources/cetech/develop/develop_system.c

        engine/sources/cetech/application/app.c
        engine/sources/cetech/task/task.c

        engine/sources/cetech/resource/resource.c
        engine/sources/cetech/resource/resource_package.c
        engine/sources/cetech/resource/resource_compiler.c
        engine/sources/cetech/filesystem/filesystem.c
        engine/sources/cetech/component/component.c
        engine/sources/cetech/entity/entity.c
        engine/sources/cetech/world/world.c
        engine/sources/cetech/level/level.c

        engine/sources/cetech/machine/machine.c
        engine/sources/cetech/machine/sdl2/sdl_mouse.c
        engine/sources/cetech/machine/sdl2/sdl_keyboard.c
        engine/sources/cetech/machine/sdl2/sdl_gamepad.c
        engine/sources/cetech/machine/sdl2/sdl_machine.c

        engine/sources/cetech/input/keyboard.c
        engine/sources/cetech/input/mouse.c
        engine/sources/cetech/input/input.c


        engine/sources/cetech/renderer/renderer.c
        engine/sources/cetech/renderer/texture/texture.c
        engine/sources/cetech/renderer/shader/shader.c
        engine/sources/cetech/renderer/material/material.c
        engine/sources/cetech/renderer/scene/scene.c
        engine/sources/cetech/renderer/mesh/mesh_renderer.c

        engine/sources/cetech/transform/transform.c
        engine/sources/cetech/scenegraph/scenegraph.c
        engine/sources/cetech/camera/camera.c

        engine/sources/cetech/hash/hash.c
        engine/sources/cetech/sdl2/sdl2.c)

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES}
        engine/sources/cetech/application/main.c)

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
        bgfxRelease.a
        bxRelease.a
        yaml-cpp.a

        luajit.a
        sqlite3.a
        SDL2.a
        assimp.a
        minizip.a
        aes.a
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

