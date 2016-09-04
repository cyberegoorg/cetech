################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include_directories(engine/sources)

add_definitions(-DCETECH_STATIC_PLUGINS)

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
endif ()

link_directories(externals/build/${PLATFORM_ID}/release/lib/)

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
        externals/build/${PLATFORM_ID}/release/include/mpack/mpack-writer.c
        engine/sources/celib/os/thread_types.h)

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        ########################################################################
        # CELIB: Containers
        ########################################################################
        engine/sources/celib/containers/tests/test_array.cc
        engine/sources/celib/containers/tests/test_hash.cc

        ########################################################################
        # CELIB: Memory
        ########################################################################
        engine/sources/celib/memory/private/allocator.c
        engine/sources/celib/memory/private/memory.c
        engine/sources/celib/os/tests/test_memory.cc


        ########################################################################
        # CELIB: string
        ########################################################################
        engine/sources/celib/string/tests/test_string.cc
        engine/sources/celib/string/private/string.c


        ########################################################################
        # CELIB: Log
        #######################################################################
        engine/sources/celib/log/private/stdout_handler.c
        engine/sources/celib/log/private/nanomsg_handler.c
        engine/sources/celib/log/private/log_system.c
        engine/sources/celib/log/tests/test_log.cc

        ########################################################################
        # CELIB: File
        ########################################################################
        engine/sources/celib/os/private/vio.c
        engine/sources/celib/os/private/sdl_vio.c
        engine/sources/celib/os/private/file.c
        engine/sources/celib/os/private/posix_stacktrace.c
        engine/sources/celib/os/tests/test_vfile.cc

        ########################################################################
        # CELIB: Crypto
        ########################################################################
        engine/sources/celib/crypto/private/hash.c
        engine/sources/celib/crypto/tests/test_murmur.cc

        ########################################################################
        # CELIB: Math
        ########################################################################
        engine/sources/celib/math/tests/test_vec2f.cc
        engine/sources/celib/math/tests/test_vec3f.cc
        engine/sources/celib/math/tests/test_vec4f.cc
        engine/sources/celib/math/tests/test_quatf.cc
        engine/sources/celib/math/tests/test_mat44f.cc

        ########################################################################
        # CELIB: Yaml
        ########################################################################
        engine/sources/celib/yaml/private/yaml.cc
        engine/sources/celib/yaml/tests/test_yaml.cc

        ########################################################################
        # CETECH: Plugin system
        ########################################################################
        engine/sources/engine/plugin_system/private/plugin_system.c

        engine/sources/engine/plugin_system/private/plugin_system.c
        engine/sources/celib/log/private/log_system.c

        engine/sources/engine/lua_system/private/lua_system.c
        engine/sources/engine/lua_system/private/api/api_keyboard.c
        engine/sources/engine/lua_system/private/api/api_log.c
        engine/sources/engine/lua_system/private/api/api_mouse.c
        engine/sources/engine/lua_system/private/api/api_application.c
        engine/sources/engine/lua_system/private/api/api_resouce_compilator.c

        engine/sources/engine/config_system/private/config_system.c
        engine/sources/engine/console_server/private/console_server.c
        engine/sources/engine/memory_system/private/memory_system.c
        engine/sources/celib/stringid/private/stringid.c
        engine/sources/engine/filesystem/private/filesystem.c
        engine/sources/engine/filesystem/tests/test_filesystem.cc
        engine/sources/celib/os/private/sdl_thread.c
        engine/sources/celib/os/private/sdl_cpu.c


        engine/sources/engine/application/private/application.c
        engine/sources/engine/machine/private/machine.c
        engine/sources/engine/machine/private/sdl2/sdl_mouse.c
        engine/sources/engine/machine/private/sdl2/sdl_keyboard.c
        engine/sources/engine/machine/private/sdl2/sdl_machine.c
        engine/sources/celib/window/private/sdl_window.c
        engine/sources/engine/input/private/keyboard.c
        engine/sources/engine/input/private/mouse.c
        engine/sources/engine/task_manager/private/task_manager.c

        engine/sources/engine/resource_compiler/private/resource_compiler.c

        engine/sources/celib/os/private/sdl_time.c
        engine/sources/celib/os/private/sdl_object.c
        engine/sources/celib/os/private/path.c
        engine/sources/engine/resource_manager/private/resource_manager..c
        engine/sources/engine/resource_manager/private/package.c)

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES}
        engine/sources/engine/main.c)

################################################################################
# Tests sources
################################################################################
set(TEST_SOURCE_FILES
        ${SOURCE_FILES}
        engine/sources/test/tests_main.cc
        engine/sources/celib/os/stacktrace.h engine/sources/celib/os/thread.h)

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
        yaml-cpp.a

        luajit.a
        sqlite3.a
        SDL2.a
        )

################################################################################
# Develop sources
################################################################################
set(DEVELOP_LIBS
        ${RELEASE_LIBS}
        assimp.a
        )

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
target_compile_definitions(cetech_develop PUBLIC -DCETECH_DEVELOP)
target_link_libraries(cetech_develop ${DEVELOP_LIBS})
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
target_include_directories(cetech_develop PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)


################################################################################
# Cetech TESTS
################################################################################
add_executable(cetech_tests ${TEST_SOURCE_FILES})
target_compile_definitions(cetech_tests PUBLIC -DCETECH_TEST -DCELIB_TEST -DCETECH_DEVELOP)
target_link_libraries(cetech_tests ${DEVELOP_LIBS})
target_include_directories(cetech_tests PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/)
target_include_directories(cetech_tests PUBLIC externals/build/${PLATFORM_ID}/${CONFIGURATION}/include)

# configure unit tests via CTest
enable_testing()
add_test(NAME RunTests COMMAND cetech_tests)

add_test(NAME ListTests COMMAND cetech_tests --list-tests)
set_tests_properties(ListTests PROPERTIES PASS_REGULAR_EXPRESSION "[0-9]+ test cases")

add_test(NAME ListTags COMMAND cetech_tests --list-tags)
set_tests_properties(ListTags PROPERTIES PASS_REGULAR_EXPRESSION "[0-9]+ tags")

add_library(plugin_example SHARED engine/sources/engine/plugin_example.c)
set_target_properties(plugin_example PROPERTIES PREFIX "")