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
        src/cetech/core/memory/private/allocator_core.c
        src/cetech/core/api/private/api_system.c
        src/cetech/core/log/private/log_system.c
        src/cetech/core/module/private/module.cpp
        src/cetech/core/config/private/config.c
        src/cetech/core/memory/private/memory.c
        src/cetech/core/memory/private/allocator_malloc.c
        src/cetech/engine/application/private/application.cpp
        src/cetech/core/task/private/task.c
        src/cetech/core/log/private/log_stdout_handler.c
        src/cetech/engine/kernel/private/kernel.c
        src/cetech/core/hashlib/private/hashlib.c
        src/cetech/core/os/private/error.c
        src/cetech/core/os/private/object.c
        src/cetech/core/os/private/path.cpp
        src/cetech/core/os/private/process.c
        src/cetech/core/os/private/cpu.c
        src/cetech/core/os/private/thread.c
        src/cetech/core/os/private/vio.c
        src/cetech/core/os/private/watchdog.cpp
        src/cetech/core/os/private/time.c
        src/cetech/engine/resource/private/resource.c
        src/cetech/engine/resource/private/resource_package.c
        src/cetech/engine/resource/private/resource_compiler.cpp
        src/cetech/core/fs/private/fs.cpp
        src/cetech/core/yaml/private/yamlng.c
        src/cetech/core/yaml/private/ydb.cpp
        src/cetech/core/cdb/private/cdb.c

        src/cetech/engine/debugui/private/bgfx_imgui/imgui.cpp
        src/cetech/engine/debugui/private/ocornut-imgui/imgui.cpp
        src/cetech/engine/debugui/private/ocornut-imgui/imgui_draw.cpp
        src/cetech/engine/debugui/private/debugui.cpp

        src/cetech/core/os/private/window_sdl2.cpp
        src/cetech/engine/machine/private/machine_sdl2.cpp
        src/cetech/engine/machine/private/mouse_sdl2.cpp
        src/cetech/engine/machine/private/keyboard_sdl2.cpp
        src/cetech/engine/machine/private/gamepad_sdl2.cpp

        src/cetech/engine/entity/private/component.cpp
        src/cetech/engine/entity/private/entity_preview.c
        src/cetech/engine/entity/private/level_preview.c
        src/cetech/engine/entity/private/entity.c
        src/cetech/engine/entity/private/world.cpp
        src/cetech/engine/level/private/level.cpp
        src/cetech/engine/input/private/keyboard.c
        src/cetech/engine/input/private/mouse.c
        src/cetech/engine/input/private/gamepad.c
        src/cetech/engine/renderer/private/renderer.cpp
        src/cetech/engine/renderer/private/texture/texture.cpp
        src/cetech/engine/renderer/private/texture/texture_compiler.cpp

        src/cetech/engine/renderer/private/shader/shader.cpp
        src/cetech/engine/renderer/private/shader/shader_compiler.cpp
        src/cetech/engine/renderer/private/material/material.cpp
        src/cetech/engine/renderer/private/material/material_preview.cpp
        src/cetech/engine/renderer/private/material/material_compiler.cpp
        src/cetech/engine/renderer/private/material/material_property.cpp
        src/cetech/engine/renderer/private/scene/scene.cpp
        src/cetech/engine/renderer/private/scene/scene_compiler.cpp
        src/cetech/engine/renderer/private/viewport/viewport.cpp
        src/cetech/engine/renderer/private/viewport/fullscreen_pass.cpp
        src/cetech/engine/renderer/private/viewport/geometry_pass.cpp
        src/cetech/engine/renderer/private/mesh_renderer/mesh_renderer.cpp
        src/cetech/engine/renderer/private/mesh_renderer/mesh_property.cpp
        src/cetech/engine/transform/private/transform.cpp

        src/cetech/engine/scenegraph/private/scenegraph.cpp
        src/cetech/engine/camera/private/camera.c

        src/cetech/engine/transform/private/transform_property.cpp
        src/cetech/engine/renderer/private/texture/texture_property.cpp

        src/cetech/playground/private/level_editor.cpp
        src/cetech/playground/private/entity_property.cpp
        src/cetech/playground/private/explorer.cpp
        src/cetech/playground/private/asset_preview.cpp
        src/cetech/playground/private/asset_property.cpp
        src/cetech/playground/private/asset_browser.cpp
        src/cetech/playground/private/playground.cpp
        src/cetech/playground/private/property_editor.cpp
        src/cetech/playground/private/command_system.cpp
        src/cetech/playground/private/command_history.cpp
        src/cetech/playground/private/log_view.cpp
        src/cetech/playground/private/action_manager.c
        src/cetech/core/math/private/fmath.c src/cetech/core/core.c)

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES} src/cetech/core/math/private/fmath.c)

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

add_library(example SHARED examples/module_example/example.cpp)
set_target_properties(example PROPERTIES PREFIX "module_")


