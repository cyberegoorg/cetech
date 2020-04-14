################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Werror -Wall")
set(EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic")

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
        )

set(CORELIB_SOURCE_FILES
        ########################################################################
        # Core
        ########################################################################
        src/celib/private/api.c
        src/celib/private/log.c
        src/celib/private/celib_module.c
        src/celib/private/config.c
        src/celib/memory/private/memory.c
        src/celib/memory/private/memory_tracer.c
        src/celib/private/task.c
        src/celib/private/log_stdout.c
        src/celib/private/id.c
        src/celib/os/private/os.c
        src/celib/os/private/os_error.c
        src/celib/os/private/os_object.c
        src/celib/os/private/os_path.c
        src/celib/os/private/os_process.c
        src/celib/os/private/os_cpu.c
        src/celib/os/private/os_thread.c
        src/celib/os/private/os_vio.c
        src/celib/os/private/os_time.c
        src/celib/os/private/os_window_sdl2.c
        src/celib/os/private/os_input_sdl.c
        src/celib/private/fs.c
        src/celib/private/yaml_cdb.c
        src/celib/private/cdb.c
        src/celib/private/core.c
        )

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        src/cetech/asset/private/asset.c
        src/cetech/asset_io/private/asset_io.c
        src/cetech/kernel/private/kernel.c
        src/cetech/ui_imgui/private/bgfx_imgui/imgui.cpp
        src/cetech/ui_imgui/private/ocornut-imgui/imgui.cpp
        src/cetech/ui_imgui/private/ocornut-imgui/imgui_draw.cpp

        src/cetech/machine/private/machine_sdl2.c

        src/cetech/ecs/private/ecs.c
        src/cetech/controlers/private/keyboard.c
        src/cetech/controlers/private/controlers.c
        src/cetech/controlers/private/mouse.c
        src/cetech/controlers/private/gamepad.c
        src/cetech/renderer/private/renderer.c
        src/cetech/render_graph/private/render_graph.c
        src/cetech/debugdraw/private/bgfx_debugdraw/debugdraw.cpp
        src/cetech/debugdraw/private/bgfx_debugdraw/bounds.cpp
        src/cetech/debugdraw/private/dd.cpp
        src/cetech/default_rg/private/default_rg.c
        src/cetech/texture/private/texture.c

        src/cetech/shader/private/shader.c
        src/cetech/material/private/material.c
        src/cetech/scene/private/scene.c
        src/cetech/scene/private/scene_compiler.cpp
        src/cetech/mesh/private/static_mesh.c
        src/cetech/mesh/private/static_mesh.c
        src/cetech/mesh/private/primitive_mesh.c
        src/cetech/transform/private/transform.c
        src/cetech/camera/private/camera.c
        src/cetech/entity/private/entity_editor.c
        src/cetech/asset_editor/private/asset_editor.c
        src/cetech/entity/private/entity_property.c
        src/cetech/explorer/private/explorer.c
        src/cetech/asset_preview/private/asset_preview.c
        src/cetech/asset_browser/private/asset_browser.cpp
        src/cetech/editor/private/editor.c
        src/cetech/property_editor/private/property_editor.c
        src/cetech/log_view/private/log_view.cpp
        src/cetech/editor/private/action_manager.c
        src/cetech/game/private/game_system.c
        src/cetech/game/private/game_view.c
        src/cetech/entity/private/entity_explorer.c
        src/cetech/editor/private/selected_object.c
        src/cetech/editor/private/dock.c
        src/cetech/node_graph/private/node_graph.c
        src/cetech/node_graph/private/node_test.c
        src/cetech/node_graph_editor/private/node_graph_editor.cpp

        src/cetech/physics2d/private/physics2d.c
        src/cetech/physics2d_box2d/private/physics2d_box2d.cpp

        src/cetech/physics3d/private/physics3d.c
        src/cetech/physics3d_bullet/private/physics3d_bullet.cpp

        src/cetech/parent/private/parent.c
        src/cetech/metrics/private/metrics.c

        src/cetech/ui_imgui/private/ui_imgui.cpp

        #        src/cetech/mono/private/mono.c
        )

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

        bgfxRelease.a
        bimgRelease.a
        bxRelease.a

        nfd.a

        SDL2.a
        yaml.a
        box2d.a
        #        monosgen-2.0.a
        celib

        Bullet2FileLoader.a
        Bullet3Collision.a
        Bullet3Common.a
        Bullet3Dynamics.a
        Bullet3Geometry.a
        Bullet3OpenCL_clew.a
        BulletCollision.a
        BulletDynamics.a
        BulletInverseDynamics.a
        BulletSoftBody.a
        clsocket.a
        LinearMath.a
        )

################################################################################
# Develop sources
################################################################################
set(DEVELOP_LIBS
        ${RELEASE_LIBS}

        )

include_directories(externals/build/${PLATFORM_ID}/release/include)
include_directories(externals/build/${PLATFORM_ID}/release/include/bullet)

#include_directories(/usr/local/include/mono-2.0)

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
target_include_directories(doc PUBLIC externals/build/${PLATFORM_ID}/release/)

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

add_library(example_develop SHARED examples/develop/src/game_example.c)
set_target_properties(example_develop PROPERTIES PREFIX "game_")

add_library(profiler SHARED src/cetech/profiler/private/profiler_dock.c)
set_target_properties(profiler PROPERTIES PREFIX "module_")
