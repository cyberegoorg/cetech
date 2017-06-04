################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include_directories(engine/include)

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
        # Core
        ########################################################################
        engine/src/core/api/api.cpp
        engine/src/core/log/log_system.cpp
        engine/src/core/os/os.cpp
        engine/src/core/os/stacktrace.cpp
        engine/src/core/yaml/yaml.cpp
        engine/src/core/module/module.cpp
        engine/src/core/config/config.cpp
        engine/src/core/memory/memory.cpp
        engine/src/core/machine/machine.cpp
        engine/src/core/machine_sdl2/sdl_mouse.cpp
        engine/src/core/machine_sdl2/sdl_keyboard.cpp
        engine/src/core/machine_sdl2/sdl_gamepad.cpp
        engine/src/core/machine_sdl2/sdl_machine.cpp


        ########################################################################
        # Modules
        ########################################################################

        engine/src/modules/luasys/luasys.c
        engine/src/modules/luasys/api/api_plugin.c
        engine/src/modules/luasys/api/api_keyboard.c
        engine/src/modules/luasys/api/api_log.c
        engine/src/modules/luasys/api/api_mouse.c
        engine/src/modules/luasys/api/api_gamepad.c
        engine/src/modules/luasys/api/api_application.c
        engine/src/modules/luasys/api/api_resouce_manager.c
        engine/src/modules/luasys/api/api_renderer.c
        engine/src/modules/luasys/api/api_world.c
        engine/src/modules/luasys/api/api_entity.c
        engine/src/modules/luasys/api/api_component.c
        engine/src/modules/luasys/api/api_transform.c
        engine/src/modules/luasys/api/api_vec2f.c
        engine/src/modules/luasys/api/api_vec3f.c
        engine/src/modules/luasys/api/api_vec4f.c
        engine/src/modules/luasys/api/api_quatf.c
        engine/src/modules/luasys/api/api_mat44f.c
        engine/src/modules/luasys/api/api_mesh.c
        engine/src/modules/luasys/api/api_material.c
        engine/src/modules/luasys/api/api_package.c
        engine/src/modules/luasys/api/api_level.c
        engine/src/modules/luasys/api/api_scenegraph.c

        engine/src/modules/console_server/console_server.cpp
        engine/src/modules/develop_system/develop_system.cpp


        engine/src/modules/resource/resource.cpp
        engine/src/modules/resource/resource_package.cpp
        engine/src/modules/resource/resource_compiler.cpp

        engine/src/modules/filesystem/filesystem.cpp
        engine/src/modules/component/component.cpp
        engine/src/modules/entity/entity.cpp
        engine/src/modules/world/world.cpp
        engine/src/modules/task/task.cpp
        engine/src/modules/level/level.cpp

        engine/src/modules/input/keyboard.cpp
        engine/src/modules/input/mouse.cpp
        engine/src/modules/input/gamepad.cpp

        engine/src/modules/renderer_bgfx/renderer.cpp
        engine/src/modules/renderer_bgfx/texture.cpp
        engine/src/modules/renderer_bgfx/shader.cpp
        engine/src/modules/renderer_bgfx/material.cpp
        engine/src/modules/renderer_bgfx/scene.cpp
        engine/src/modules/renderer_bgfx/mesh_renderer.cpp

        engine/src/modules/transform/transform.cpp
        engine/src/modules/scenegraph/scenegraph.cpp
        engine/src/modules/camera/camera.cpp
        )

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES}
        engine/src/core/app/application.cpp)

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

add_library(module_example SHARED engine/examples/module_example/example.c)
set_target_properties(module_example PROPERTIES PREFIX "")

