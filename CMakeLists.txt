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
    add_definitions(-DDEBUG)
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
        src/sqlite/private/sqlite3.c
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
        src/celib/private/api_system.c
        src/celib/private/log.c
        src/celib/private/module.c
        src/celib/private/config.c
        src/celib/private/memory.c
        src/celib/private/task.c
        src/celib/private/log_stdout.c
        src/celib/private/hashlib.c
        src/celib/private/os.c
        src/celib/private/os_error.c
        src/celib/private/os_object.c
        src/celib/private/os_path.c
        src/celib/private/os_process.c
        src/celib/private/os_cpu.c
        src/celib/private/os_thread.c
        src/celib/private/os_vio.c
        src/celib/private/os_time.c
        src/celib/private/os_window_sdl2.c
        src/celib/private/fs.c
        src/celib/private/ydb.c
        src/celib/private/cdb.c
        src/celib/private/core.c
        )

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        src/cetech/resource/private/resource.c
        src/cetech/resource/private/resource_compiler.c
        src/cetech/kernel/private/kernel.c
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
        src/cetech/renderer/private/renderer.c
        src/cetech/render_graph/private/render_graph.c
        src/cetech/default_rg/private/default_rg.c
        src/cetech/texture/private/texture.c

        src/cetech/shader/private/shader.c
        src/cetech/material/private/material.c
        src/cetech/scene/private/scene.c
        src/cetech/scene/private/scene_compiler.cpp
        src/cetech/mesh/private/mesh_renderer.c
        src/cetech/transform/private/transform.c
        src/cetech/camera/private/camera.c
        src/cetech/entity/private/entity_editor.c
        src/cetech/editor/private/resource_editor.c
        src/cetech/entity/private/entity_property.c
        src/cetech/editor/private/explorer.c
        src/cetech/editor/private/resource_preview.c
        src/cetech/editor/private/resource_browser.cpp
        src/cetech/editor/private/editor.c
        src/cetech/editor/private/property.c
        src/cetech/editor/private/log_view.cpp
        src/cetech/editor/private/action_manager.c
        src/cetech/resource/private/builddb.c
        src/cetech/editor/private/resource_ui.c
        src/cetech/game/private/game_system.c
        src/cetech/game/private/game_view.c
        src/cetech/entity/private/entity_explorer.c
        src/cetech/editor/private/selected_object.c
        src/cetech/editor/private/dock.c)

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
            "-framework Metal"
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

        SDL2.a

        assimp.a
        IrrXML.a
        minizip.a
        yaml_static.a
        #aes.a

        celib
        )

set(RELEASE_LIBS2
        ${RELEASE_LIBS_LINUX}
        ${RELEASE_LIBS_WINDOWS}

        nanomsg
        bgfx-shared-libRelease
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

add_library(celib STATIC ${CORELIB_SOURCE_FILES})
target_include_directories(celib PUBLIC externals/build/${PLATFORM_ID}/release/)

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

add_library(example_develop SHARED examples/develop/src/game_example.c)
set_target_properties(example_develop PROPERTIES PREFIX "game_")
