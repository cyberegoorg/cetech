################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include_directories(src)

add_definitions(-DCETECH_STATIC_PLUGINS)
add_definitions(-DCETECH_USE_SDL)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONFIGURATION debug)
    add_definitions(-DCETECH_DEBUG)
else ()
    set(CONFIGURATION release)
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

        src/cetech/modules/debugui/private/bgfx_imgui/imgui.cpp
        src/cetech/modules/debugui/private/ocornut-imgui/imgui.cpp
        src/cetech/modules/debugui/private/ocornut-imgui/imgui_draw.cpp
        src/cetech/modules/debugui/private/debugui.cpp

        src/cetech/modules/playground/private/playground.cpp
        src/cetech/modules/playground/private/level_editor.cpp

        src/cetech/engine/machine/private/window_sdl2.cpp
        src/cetech/engine/machine/private/machine_sdl2.cpp
        src/cetech/engine/machine/private/mouse_sdl2.cpp
        src/cetech/engine/machine/private/keyboard_sdl2.cpp
        src/cetech/engine/machine/private/gamepad_sdl2.cpp

        src/cetech/core/memory/private/allocator_core.cpp
        src/cetech/core/api/private/api_system.cpp
        src/cetech/core/log/private/log_system.cpp
        src/cetech/core/yaml/private/yaml.cpp
        src/cetech/core/module/private/module.cpp
        src/cetech/core/config/private/config.cpp
        src/cetech/core/memory/private/memory.cpp
        src/cetech/core/memory/private/allocator_malloc.cpp
        src/cetech/core/memory/private/allocator_scratch.cpp
        src/cetech/engine/application/private/application.cpp
        src/cetech/core/task/private/task.cpp
        src/cetech/core/log/private/log_db_handler.cpp
        src/cetech/core/log/private/log_nanomsg_handler.cpp
        src/cetech/core/log/private/log_stdout_handler.cpp
        src/cetech/engine/kernel/private/kernel.cpp
        src/cetech/core/hashlib/private/hashlib.cpp

        src/cetech/core/os/private/error.cpp
        src/cetech/core/os/private/object.cpp
        src/cetech/core/os/private/path.cpp
        src/cetech/core/os/private/process.cpp
        src/cetech/core/os/private/cpu.cpp
        src/cetech/core/os/private/thread.cpp
        src/cetech/core/os/private/vio.cpp
        src/cetech/core/os/private/watchdog.cpp
        src/cetech/core/os/private/time.cpp

        ########################################################################
        # Modules
        ########################################################################

        src/cetech/modules/luasys/private/luasys_private.cpp
        src/cetech/modules/luasys/private/api/api_log.cpp
        src/cetech/modules/luasys/private/api/api_vec3f.cpp
        src/cetech/modules/luasys/private/api/api_quatf.cpp
        src/cetech/modules/luasys/private/api/api_mat44f.cpp

        src/cetech/engine/resource/private/resource.cpp
        src/cetech/engine/resource/private/resource_package.cpp
        src/cetech/engine/resource/private/resource_compiler.cpp

        src/cetech/engine/filesystem/private/filesystem.cpp
        src/cetech/engine/entity/private/component.cpp
        src/cetech/engine/entity/private/entity.cpp
        src/cetech/engine/entity/private/world.cpp
        src/cetech/modules/level/private/level.cpp

        src/cetech/engine/input/private/keyboard.cpp
        src/cetech/engine/input/private/mouse.cpp
        src/cetech/engine/input/private/gamepad.cpp

        src/cetech/modules/renderer/private/renderer.cpp
        src/cetech/modules/renderer/private/texture/texture.cpp
        src/cetech/modules/renderer/private/shader/shader.cpp
        src/cetech/modules/renderer/private/material/material.cpp
        src/cetech/modules/renderer/private/scene/scene.cpp
        src/cetech/modules/renderer/private/mesh_renderer/mesh_renderer.cpp
        src/cetech/modules/renderer/private/mesh_renderer/mesh_renderer.cpp

        src/cetech/modules/transform/private/transform.cpp
        src/cetech/modules/scenegraph/private/scenegraph.cpp
        src/cetech/modules/camera/private/camera.cpp
        src/cetech/core/blob/private/blob.cpp
        src/cetech/modules/renderer/private/scene/scene_compiler.cpp
        src/cetech/modules/renderer/private/shader/shader_compiler.cpp
        src/cetech/modules/renderer/private/texture/texture_compiler.cpp
        src/cetech/modules/renderer/private/material/material_compiler.cpp
        src/cetech/modules/renderer/private/material/material_compiler.cpp

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

#add_library(cetech_develop_lib SHARED ${SOURCE_FILES})
#target_link_libraries(cetech_develop_lib ${RELEASE_LIBS})
#target_compile_definitions(cetech_develop_lib PUBLIC -DCETECH_DEVELOP -DCETECH_CAN_COMPILE=1)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

add_library(module_example SHARED examples/module_example/example.cpp)
set_target_properties(module_example PROPERTIES PREFIX "")

