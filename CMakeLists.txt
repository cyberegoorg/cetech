################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")


include_directories(engine/sources)

add_definitions(-DCETECH_STATIC_PLUGINS)
add_definitions(-DCETECH_USE_SDL)

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
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-writer.c)

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        ########################################################################
        # CETECH
        ########################################################################
        engine/sources/core_private/log/log_system.cpp


        engine/sources/core_private/os/stacktrace.cpp
        engine/sources/core_private/api/api.cpp
        engine/sources/core_private/log/log_system.cpp
        engine/sources/core_private/yaml/yaml.cpp


        ########################################################################
        # CETECH
        ########################################################################
        engine/sources/core_private/module/module.cpp
        engine/sources/core_private/config/config.cpp
        engine/sources/core_private/memory/memory.cpp

        engine/sources/cetech/modules/luasys/private/luasys.c
        engine/sources/cetech/modules/luasys/private/api/api_plugin.c
        engine/sources/cetech/modules/luasys/private/api/api_keyboard.c
        engine/sources/cetech/modules/luasys/private/api/api_log.c
        engine/sources/cetech/modules/luasys/private/api/api_mouse.c
        engine/sources/cetech/modules/luasys/private/api/api_gamepad.c
        engine/sources/cetech/modules/luasys/private/api/api_application.c
        engine/sources/cetech/modules/luasys/private/api/api_resouce_manager.c
        engine/sources/cetech/modules/luasys/private/api/api_renderer.c
        engine/sources/cetech/modules/luasys/private/api/api_world.c
        engine/sources/cetech/modules/luasys/private/api/api_entity.c
        engine/sources/cetech/modules/luasys/private/api/api_component.c
        engine/sources/cetech/modules/luasys/private/api/api_transform.c
        engine/sources/cetech/modules/luasys/private/api/api_vec2f.c
        engine/sources/cetech/modules/luasys/private/api/api_vec3f.c
        engine/sources/cetech/modules/luasys/private/api/api_vec4f.c
        engine/sources/cetech/modules/luasys/private/api/api_quatf.c
        engine/sources/cetech/modules/luasys/private/api/api_mat44f.c
        engine/sources/cetech/modules/luasys/private/api/api_mesh.c
        engine/sources/cetech/modules/luasys/private/api/api_material.c
        engine/sources/cetech/modules/luasys/private/api/api_package.c
        engine/sources/cetech/modules/luasys/private/api/api_level.c
        engine/sources/cetech/modules/luasys/private/api/api_scenegraph.c

        engine/sources/cetech/modules/develop_system/private/console_server.cpp
        engine/sources/cetech/modules/develop_system/private/develop_system.cpp

        engine/sources/cetech/modules/task/private/task.cpp

        engine/sources/cetech/modules/resource/private/resource.cpp
        engine/sources/cetech/modules/resource/private/resource_package.cpp
        engine/sources/cetech/modules/resource/private/resource_compiler.cpp
        engine/sources/cetech/modules/filesystem/private/filesystem.cpp
        engine/sources/cetech/modules/component/private/component.cpp
        engine/sources/cetech/modules/entity/private/entity.cpp
        engine/sources/cetech/modules/world/private/world.cpp
        engine/sources/cetech/modules/level/private/level.cpp

        engine/sources/core_private/machine/machine.cpp
        engine/sources/core_private/machine_sdl2/sdl_mouse.cpp
        engine/sources/core_private/machine_sdl2/sdl_keyboard.cpp
        engine/sources/core_private/machine_sdl2/sdl_gamepad.cpp
        engine/sources/core_private/machine_sdl2/sdl_machine.cpp

        engine/sources/cetech/modules/input/private/keyboard.cpp
        engine/sources/cetech/modules/input/private/mouse.cpp
        engine/sources/cetech/modules/input/private/gamepad.cpp


        engine/sources/cetech/modules/renderer_bgfx/private/renderer.cpp
        engine/sources/cetech/modules/renderer_bgfx/private/texture.cpp
        engine/sources/cetech/modules/renderer_bgfx/private/shader.cpp
        engine/sources/cetech/modules/renderer_bgfx/private/material.cpp
        engine/sources/cetech/modules/renderer_bgfx/private/scene.cpp
        engine/sources/cetech/modules/renderer_bgfx/private/mesh_renderer.cpp

        engine/sources/cetech/modules/transform/private/transform.cpp
        engine/sources/cetech/modules/scenegraph/private/scenegraph.cpp
        engine/sources/cetech/modules/camera/private/camera.cpp

        engine/sources/core_private/os/os.cpp)

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES}
        engine/sources/core_private/app/application.cpp)

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
target_compile_definitions(cetech_develop PUBLIC -DCETECH_DEVELOP -DCETECH_CAN_COMPILE=1)
target_link_libraries(cetech_develop ${DEVELOP_LIBS})
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

#add_library(cetech_develop_lib SHARED ${SOURCE_FILES})
#target_link_libraries(cetech_develop_lib ${RELEASE_LIBS2})
#target_compile_definitions(cetech_develop_lib PUBLIC -DCETECH_DEVELOP)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

#set_target_properties(module_example PROPERTIES PREFIX "")

add_library(module_example SHARED engine/sources/module_example/example.c)
set_target_properties(module_example PROPERTIES PREFIX "")

