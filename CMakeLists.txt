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
        engine/src/kernel/api/api_system.cpp
        engine/src/kernel/log/log_system.cpp
        engine/src/kernel/os_sdl2/os.cpp
        engine/src/kernel/os_sdl2/sdl2_stacktrace.cpp
        engine/src/kernel/yaml/yaml.cpp
        engine/src/kernel/module/module.cpp
        engine/src/kernel/config/config.cpp
        engine/src/kernel/memory/memory.cpp
        engine/src/kernel/os_sdl2/machine.cpp
        engine/src/kernel/os_sdl2/sdl2_mouse.cpp
        engine/src/kernel/os_sdl2/sdl2_keyboard.cpp
        engine/src/kernel/os_sdl2/sdl2_gamepad.cpp
        engine/src/kernel/application/application.cpp
        engine/src/kernel/task/task.cpp
        engine/src/kernel/develop_system/develop_system.cpp
        engine/src/kernel/log/log_db_handler.cpp
        engine/src/kernel/log/log_nanomsg_handler.cpp
        engine/src/kernel/log/log_stdout_handler.cpp
        engine/src/kernel/kernel/kernel.cpp
        engine/src/kernel/hash/hashlib.cpp

        ########################################################################
        # Modules
        ########################################################################

        engine/src/modules/luasys/luasys.cpp
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


        engine/src/modules/resource/resource.cpp
        engine/src/modules/resource/resource_package.cpp
        engine/src/modules/resource/resource_compiler.cpp

        engine/src/modules/filesystem/filesystem.cpp
        engine/src/modules/entity/component.cpp
        engine/src/modules/entity/entity.cpp
        engine/src/modules/entity/world.cpp
        engine/src/modules/level/level.cpp

        engine/src/modules/input/keyboard.cpp
        engine/src/modules/input/mouse.cpp
        engine/src/modules/input/gamepad.cpp

        engine/src/modules/renderer_bgfx/renderer.cpp
        engine/src/modules/renderer_bgfx/texture/texture.cpp
        engine/src/modules/renderer_bgfx/shader/shader.cpp
        engine/src/modules/renderer_bgfx/material/material.cpp
        engine/src/modules/renderer_bgfx/scene/scene.cpp
        engine/src/modules/renderer_bgfx/mesh_renderer/mesh_renderer.cpp
        engine/src/modules/renderer_bgfx/mesh_renderer/mesh_renderer.cpp

        engine/src/modules/transform/transform.cpp
        engine/src/modules/scenegraph/scenegraph.cpp
        engine/src/modules/camera/camera.cpp
        engine/src/modules/blob/blob.cpp
        engine/src/modules/renderer_bgfx/scene/scene_compiler.cpp
        engine/src/modules/renderer_bgfx/shader/shader_compiler.cpp
        engine/src/modules/renderer_bgfx/texture/texture_compiler.cpp
        engine/src/modules/renderer_bgfx/material/material_compiler.cpp
        engine/src/modules/renderer_bgfx/material/material_compiler.cpp

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
        yaml-cpp.a

        bgfxRelease.a
        bxRelease.a

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
#target_compile_definitions(cetech_develop_lib PUBLIC -DCETECH_DEVELOP -DCETECH_CAN_COMPILE=1)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
#target_include_directories(cetech_develop_lib PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

#set_target_properties(module_example PROPERTIES PREFIX "")

add_library(module_example SHARED engine/examples/module_example/example.c)
set_target_properties(module_example PROPERTIES PREFIX "")

