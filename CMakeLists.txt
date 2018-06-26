################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror -Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include_directories(src)

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
#        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-common.c
#        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-expect.c
#        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-node.c
#        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-platform.c
#        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-reader.c
#        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-writer.c
        )

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        ########################################################################
        # Core
        ########################################################################
        src/corelib/private/allocator_core.c
        src/corelib/private/api_system.c
        src/corelib/private/log_system.c
        src/corelib/private/module.c
        src/corelib/private/config.c
        src/corelib/private/memory.c
        src/corelib/private/allocator_malloc.c
        src/corelib/private/task.c
        src/corelib/private/log_stdout_handler.c
        src/cetech/kernel/kernel.c
        src/corelib/private/hashlib.c
        src/corelib/private/os_error.c
        src/corelib/private/os_object.c
        src/corelib/private/os_path.c
        src/corelib/private/os_process.c
        src/corelib/private/os_cpu.c
        src/corelib/private/os_thread.c
        src/corelib/private/os_vio.c
        src/corelib/private/os_watchdog.c
        src/corelib/private/os_time.c
        src/cetech/resource/private/resource.c
        src/cetech/resource/private/resource_package.c
        src/cetech/resource/private/resource_compiler.c
        src/corelib/private/fs.c
        src/corelib/private/yng.c
        src/corelib/private/ydb.cpp
        src/corelib/private/cdb.c
        src/corelib/private/ebus.c

        src/cetech/debugui/private/bgfx_imgui/imgui.cpp
        src/cetech/debugui/private/ocornut-imgui/imgui.cpp
        src/cetech/debugui/private/ocornut-imgui/imgui_draw.cpp
        src/cetech/debugui/private/debugui.cpp

        src/cetech/debugdraw/private/debugdraw/debugdraw.cpp
        src/cetech/debugdraw/private/debugdraw/bounds.cpp
        src/cetech/debugdraw/private/debugdraw.cpp

        src/corelib/private/os_window_sdl2.c
        src/cetech/machine/private/machine_sdl2.c

        src/cetech/playground/private/entity_preview.c
        src/cetech/ecs/private/ecs.c
        src/cetech/controlers/private/keyboard.c
        src/cetech/controlers/private/mouse.c
        src/cetech/controlers/private/gamepad.c
        src/cetech/renderer/private/renderer.cpp
        src/cetech/render_graph/private/render_graph.c
        src/cetech/default_render_graph/private/default_render_graph.c
        src/cetech/texture/private/texture.cpp
        src/cetech/texture/private/texture_compiler.cpp

        src/cetech/shader/private/shader.cpp
        src/cetech/shader/private/shader_compiler.cpp
        src/cetech/material/private/material.cpp
        src/cetech/material/private/material_preview.cpp
        src/cetech/material/private/material_compiler.cpp
        src/cetech/material/private/material_property.cpp
        src/cetech/scene/private/scene.cpp
        src/cetech/scene/private/scene_compiler.cpp
        src/cetech/mesh_renderer/private/mesh_renderer.c
        src/cetech/transform/private/transform.c

        src/cetech/scenegraph/private/scenegraph.c
        src/cetech/camera/private/camera.c

        src/cetech/texture/private/texture_property.cpp

        src/cetech/playground/private/entity_editor.c
        src/cetech/playground/private/entity_property.c
        src/cetech/playground/private/explorer.cpp
        src/cetech/playground/private/asset_preview.c
        src/cetech/playground/private/asset_property.c
        src/cetech/playground/private/asset_browser.cpp
        src/cetech/playground/private/playground.c
        src/cetech/playground/private/property_editor.c
        src/cetech/playground/private/command_system.c
        src/cetech/playground/private/command_history.c
        src/cetech/playground/private/log_view.cpp
        src/cetech/playground/private/action_manager.c
        src/cetech/kernel/core.c)

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES} )

################################################################################
# Release sources
################################################################################

if (APPLE)
    set(RELEASE_LIBS_DARWIN
            z
            dl
            m
            iconv
            objc

            "-framework Cocoa"
            "-framework CoreAudio"
            "-framework AudioToolbox"
            "-framework ForceFeedback"
            #"-framework CoreVideo"
            "-framework Carbon"
            "-framework IOKit"
            "-framework QuartzCore"
            "-framework OpenGL"
            "-weak_framework Metal"
            )

elseif(UNIX)
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
        ${RELEASE_LIBS_DARWIN}

#        nanomsg.a
        bgfxRelease.a
        bimgRelease.a
        bxRelease.a

        sqlite3.a
        SDL2.a

        assimp.a
        IrrXML.a
        minizip.a
        yaml_static.a
        #aes.a
        )

set(RELEASE_LIBS2
        ${RELEASE_LIBS_LINUX}
        ${RELEASE_LIBS_WINDOWS}

        nanomsg
        bgfx-shared-libRelease
        sqlite3
        assimp
        SDL2
        #luajit
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
#add_executable(cetech ${MAIN_SOURCE_FILES})
#target_compile_definitions(cetech PUBLIC -DCETECH_RELEASE)
#target_link_libraries(cetech ${RELEASE_LIBS})
#target_include_directories(cetech PUBLIC externals/build/${PLATFORM_ID}/release/)
#target_include_directories(cetech PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

################################################################################
# Cetech DEVELOP
################################################################################
add_executable(cetech_develop ${MAIN_SOURCE_FILES})
target_compile_definitions(cetech_develop PUBLIC -DCETECH_DEVELOP)
target_link_libraries(cetech_develop ${DEVELOP_LIBS})
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

#add_library(cetech_develop_lib SHARED ${SOURCE_FILES})
#target_link_libraries(cetech_develop_lib ${RELEASE_LIBS})
#target_compile_definitions(cetech_develop_lib PUBLIC -DCETECH_DEVELOP -DCETECH_CAN_COMPILE=1)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

add_library(example SHARED examples/module_example/example.c)
set_target_properties(example PROPERTIES PREFIX "module_")

add_library(example_develop SHARED examples/develop/src/game.c)
set_target_properties(example_develop PROPERTIES PREFIX "game_")

