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
        engine/src/core/os/error.cpp
        engine/src/machine/machine_sdl2.cpp
        engine/src/machine/mouse_sdl2.cpp
        engine/src/machine/keyboard_sdl2.cpp
        engine/src/machine/gamepad_sdl2.cpp

        engine/src/engine/console_server/console_server.cpp
        engine/src/core/memory/allocator_core.cpp
        engine/src/core/api/api_system.cpp
        engine/src/core/log/log_system.cpp
        engine/src/core/yaml/yaml.cpp
        engine/src/core/module/module.cpp
        engine/src/core/config/config.cpp
        engine/src/core/memory/memory.cpp
        engine/src/core/memory/allocator_malloc.cpp
        engine/src/core/memory/allocator_scratch.cpp
        engine/src/engine/application/application.cpp
        engine/src/core/task/task.cpp
        engine/src/engine/develop_system/develop_system.cpp
        engine/src/core/log/log_db_handler.cpp
        engine/src/core/log/log_nanomsg_handler.cpp
        engine/src/core/log/log_stdout_handler.cpp
        engine/src/engine/kernel/kernel.cpp
        engine/src/core/hash/hashlib.cpp
        engine/src/core/os/object.cpp
        engine/src/core/os/path.cpp
        engine/src/core/os/process.cpp
        engine/src/core/os/cpu.cpp
        engine/src/core/os/thread.cpp
        engine/src/core/os/vio.cpp
        engine/src/core/os/window.cpp
        engine/src/core/os/time.cpp

        ########################################################################
        # Modules
        ########################################################################

        engine/src/engine/luasys/luasys_private.cpp
        engine/src/engine/luasys/api/api_log.cpp
        engine/src/engine/luasys/api/api_vec3f.cpp
        engine/src/engine/luasys/api/api_quatf.cpp
        engine/src/engine/luasys/api/api_mat44f.cpp

        engine/src/engine/resource/resource.cpp
        engine/src/engine/resource/resource_package.cpp
        engine/src/engine/resource/resource_compiler.cpp

        engine/src/engine/filesystem/filesystem.cpp
        engine/src/engine/entity/component.cpp
        engine/src/engine/entity/entity.cpp
        engine/src/engine/entity/world.cpp
        engine/src/engine/level/level.cpp

        engine/src/engine/input/keyboard.cpp
        engine/src/engine/input/mouse.cpp
        engine/src/engine/input/gamepad.cpp

        engine/src/engine/renderer_bgfx/renderer.cpp
        engine/src/engine/renderer_bgfx/texture/texture.cpp
        engine/src/engine/renderer_bgfx/shader/shader.cpp
        engine/src/engine/renderer_bgfx/material/material.cpp
        engine/src/engine/renderer_bgfx/scene/scene.cpp
        engine/src/engine/renderer_bgfx/mesh_renderer/mesh_renderer.cpp
        engine/src/engine/renderer_bgfx/mesh_renderer/mesh_renderer.cpp

        engine/src/engine/transform/transform.cpp
        engine/src/engine/scenegraph/scenegraph.cpp
        engine/src/engine/camera/camera.cpp
        engine/src/core/blob/blob.cpp
        engine/src/engine/renderer_bgfx/scene/scene_compiler.cpp
        engine/src/engine/renderer_bgfx/shader/shader_compiler.cpp
        engine/src/engine/renderer_bgfx/texture/texture_compiler.cpp
        engine/src/engine/renderer_bgfx/material/material_compiler.cpp
        engine/src/engine/renderer_bgfx/material/material_compiler.cpp

        )

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES})

################################################################################
# Release sources
################################################################################

if (UNIX)
    set(RELEASE_LIBS_LINUX
            #jemalloc_pic.a
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
        yaml-cpp.a

        bgfxRelease.a
        bimgRelease.a
        bxRelease.a

        luajit.a
        sqlite3.a
        SDL2.a

        assimp.a
        IrrXML.a
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

add_library(cetech_develop_lib SHARED ${SOURCE_FILES})
target_link_libraries(cetech_develop_lib ${RELEASE_LIBS})
target_compile_definitions(cetech_develop_lib PUBLIC -DCETECH_DEVELOP -DCETECH_CAN_COMPILE=1)
target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

add_library(module_example SHARED engine/examples/module_example/example.c)
set_target_properties(module_example PROPERTIES PREFIX "")

