################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

include_directories(sources)

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
        )

set(SOURCE_FILES
        ${EXTERNAL_SOURCE_FILES}

        ########################################################################
        # CELIB: Containers
        ########################################################################
        sources/celib/containers/tests/test_array.cc

        ########################################################################
        # CELIB: Memory
        ########################################################################
        sources/celib/memory/private/allocator.c
        sources/celib/memory/private/memory.c
        sources/celib/os/tests/test_memory.cc


        ########################################################################
        # CELIB: string
        ########################################################################
        sources/celib/string/tests/test_string.cc
        sources/celib/string/private/string.c


        ########################################################################
        # CELIB: Log
        #######################################################################
        sources/celib/log/private/stdout_handler.c
        sources/celib/log/private/nanomsg_handler.c
        sources/celib/log/private/log_system.c
        sources/celib/log/tests/test_log.cc

        ########################################################################
        # CELIB: File
        ########################################################################
        sources/celib/os/private/memory.c
        sources/celib/os/private/vfile.c
        sources/celib/os/private/sdl_vfile.c
        sources/celib/os/private/path.c
        sources/celib/os/private/file.c
        sources/celib/os/private/posix_stacktrace.c
        sources/celib/os/tests/test_vfile.cc

        ########################################################################
        # CELIB: Crypto
        ########################################################################
        sources/celib/crypto/private/hash.c
        sources/celib/crypto/tests/test_murmur.cc

        ########################################################################
        # CELIB: Math
        ########################################################################
        sources/celib/math/tests/test_vec2f.cc
        sources/celib/math/tests/test_vec3f.cc
        sources/celib/math/tests/test_vec4f.cc
        sources/celib/math/tests/test_quatf.cc
        sources/celib/math/tests/test_mat44f.cc

        ########################################################################
        # CELIB: Yaml
        ########################################################################
        sources/celib/yaml/private/yaml.cc
        sources/celib/yaml/tests/test_yaml.cc

        ########################################################################
        # CETECH: Plugin system
        ########################################################################
        sources/cetech/pluginsystem/private/plugin_system.c

        sources/cetech/pluginsystem/private/plugin_system.c
        sources/celib/log/private/log_system.c

        sources/cetech/luasystem/private/lua_system.c
        sources/cetech/luasystem/private/api/api_keyboard.c
        sources/cetech/luasystem/private/api/api_log.c
        sources/cetech/luasystem/private/api/api_mouse.c
        sources/cetech/luasystem/private/api/api_application.c

        sources/cetech/configsystem/private/config_system.c
        sources/cetech/consoleserver/private/console_server.c
        sources/celib/memory/private/memory_system.c
        sources/celib/stringid/private/stringid.c
        sources/cetech/filesystem/private/filesystem.c
        sources/cetech/filesystem/tests/test_filesystem.cc
        sources/celib/os/private/sdl_thread.c


        sources/cetech/application/private/application.c
        sources/cetech/machine/private/machine.c
        sources/cetech/machine/private/sdl2/sdl_mouse.c
        sources/cetech/machine/private/sdl2/sdl_keyboard.c
        sources/cetech/machine/private/sdl2/sdl_machine.c
        sources/cetech/machine/private/sdl2/sdl_window.c
        sources/cetech/input/private/keyboard.c
        sources/cetech/input/private/mouse.c


)

################################################################################
# Main sources
################################################################################
set(MAIN_SOURCE_FILES
        ${SOURCE_FILES}
        sources/cetech/main.c
        )

################################################################################
# Tests sources
################################################################################
set(TEST_SOURCE_FILES
        ${SOURCE_FILES}
        sources/tests_main.cc
        )

################################################################################
# Release sources
################################################################################

if (UNIX)
    set(RELEASE_LIBS_LINUX
            jemalloc_pic
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

        nanomsg
        bgfxRelease
        yaml-cpp

        luajit

        SDL2
        )

################################################################################
# Develop sources
################################################################################
set(DEVELOP_LIBS
        ${RELEASE_LIBS}
        assimp
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

add_library(plugin_example SHARED sources/cetech/plugin_example.c)
set_target_properties(plugin_example PROPERTIES PREFIX "")