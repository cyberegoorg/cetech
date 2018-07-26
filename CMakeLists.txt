################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror -Wall")
set(EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
set(CMAKE_SHARED_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic")

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
elseif (UNIX)
    set(PLATFORM_ID linux64)
elseif (WIN32)
    set(PLATFORM_ID windows64)

    add_definitions(-DCETECH_WINDOWS)
    add_definitions(-DCETECH_WINDOWS)
endif ()

link_directories(externals/build/${PLATFORM_ID}/release/lib/)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_ID})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_ID})



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

set(CORELIB_SOURCE_FILES
        ########################################################################
        # Core
        ########################################################################
        src/corelib/private/api_system.c
        src/corelib/private/log.c
        src/corelib/private/module.c
        src/corelib/private/config.c
        src/corelib/private/memory.c
        src/corelib/private/task.c
        src/corelib/private/log_stdout.c
        src/corelib/private/hashlib.c
        src/corelib/private/os.c
        src/corelib/private/os_error.c
        src/corelib/private/os_object.c
        src/corelib/private/os_path.c
        src/corelib/private/os_process.c
        src/corelib/private/os_cpu.c
        src/corelib/private/os_thread.c
        src/corelib/private/os_vio.c
        src/corelib/private/os_watchdog.c
        src/corelib/private/os_time.c
        src/corelib/private/os_window_sdl2.c
        src/corelib/private/fs.c
        src/corelib/private/yng.c
        src/corelib/private/ydb.c
        src/corelib/private/cdb.c
        src/corelib/private/ebus.c
        src/corelib/private/core.c
        )

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        src/cetech/resource/private/resource.c
        src/cetech/resource/private/package.c
        src/cetech/resource/private/resource_compiler.c
        src/cetech/kernel/kernel.c
        src/cetech/debugui/private/bgfx_imgui/imgui.cpp
        src/cetech/debugui/private/ocornut-imgui/imgui.cpp
        src/cetech/debugui/private/ocornut-imgui/imgui_draw.cpp
        src/cetech/debugui/private/debugui.cpp

        src/cetech/debugdraw/private/debugdraw/debugdraw.cpp
        src/cetech/debugdraw/private/debugdraw/bounds.cpp
        src/cetech/debugdraw/private/debugdraw.cpp

        src/cetech/machine/private/machine_sdl2.c

        src/cetech/ecs/private/ecs.c
        src/cetech/controlers/private/keyboard.c
        src/cetech/controlers/private/controlers.c
        src/cetech/controlers/private/mouse.c
        src/cetech/controlers/private/gamepad.c
        src/cetech/renderer/private/renderer.cpp
        src/cetech/render_graph/private/render_graph.c
        src/cetech/default_render_graph/private/default_render_graph.c
        src/cetech/texture/private/texture.c

        src/cetech/shader/private/shader.c
        src/cetech/material/private/material.cpp
        src/cetech/material/private/material_compiler.cpp
        src/cetech/scene/private/scene.c
        src/cetech/scene/private/scene_compiler.cpp
        src/cetech/mesh_renderer/private/mesh_renderer.c
        src/cetech/transform/private/transform.c

        src/cetech/scenegraph/private/scenegraph.c
        src/cetech/camera/private/camera.c

        src/cetech/entity/private/entity_editor.c
        src/cetech/asset_editor/private/asset_editor.c
        src/cetech/entity/private/entity_property.c
        src/cetech/explorer/private/explorer.c
        src/cetech/asset_preview/private/asset_preview.c
        src/cetech/asset_property/private/asset_property.c
        src/cetech/asset_browser/private/asset_browser.cpp
        src/cetech/editor/private/editor.c
        src/cetech/property_editor/private/property_editor.c
        src/cetech/command_system/private/command_system.c
        src/cetech/command_history/private/command_history.c
        src/cetech/log_view/private/log_view.cpp
        src/cetech/action_manager/private/action_manager.c
        src/cetech/selected_object/private/selected_object.c
        src/cetech/builddb/private/builddb.c
        src/cetech/editor_ui/private/editor_ui.c)

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES})

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

elseif (UNIX)
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

        corelib
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PLATFORM_ID})

################################################################################
# Cetech RUNTIME
################################################################################
#add_executable(cetech ${MAIN_SOURCE_FILES})
#target_compile_definitions(cetech PUBLIC -DCETECH_RELEASE)
#target_link_libraries(cetech ${RELEASE_LIBS})
#target_include_directories(cetech PUBLIC externals/build/${PLATFORM_ID}/release/)
#target_include_directories(cetech PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

################################################################################
# Corelib
################################################################################

add_library(corelib STATIC ${CORELIB_SOURCE_FILES})
target_include_directories(corelib PUBLIC externals/build/${PLATFORM_ID}/release/)

add_executable(doc src/tools/doc/doc.c)
target_link_libraries(doc ${DEVELOP_LIBS})
target_include_directories(doc PUBLIC externals/build/${PLATFORM_ID}/elease/)

add_executable(hash src/tools/hash/hash.c)
target_link_libraries(hash ${DEVELOP_LIBS})
target_include_directories(hash PUBLIC externals/build/${PLATFORM_ID}/release/)

################################################################################
# Cetech DEVELOP
################################################################################


add_executable(cetech_develop ${MAIN_SOURCE_FILES})
target_compile_definitions(cetech_develop PUBLIC -DCETECH_DEVELOP)
target_link_libraries(cetech_develop ${DEVELOP_LIBS})
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/release/)

add_library(cetech_develop_lib SHARED ${SOURCE_FILES})
target_link_libraries(cetech_develop_lib ${DEVELOP_LIBS})
target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/release/)

add_library(example SHARED examples/module_example/example.c)
set_target_properties(example PROPERTIES PREFIX "module_")

add_library(example_develop SHARED examples/develop/src/game.c)
set_target_properties(example_develop PROPERTIES PREFIX "game_")
