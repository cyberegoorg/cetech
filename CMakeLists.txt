################################################################################
# CETech
################################################################################
cmake_minimum_required(VERSION 3.5)
project(cetech)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic")

add_definitions( -DCETECH_COLORED_LOG )
include_directories(soureces)
link_directories(externals/build/linux64/release/lib/)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DCETECH_DEBUG)
endif()

################################################################################
# Sources
################################################################################
set(SOURCE_FILES
        ########################################################################
        # Externals: MPack
        ########################################################################
        externals/build/linux64/release/include/mpack/mpack-common.c
        externals/build/linux64/release/include/mpack/mpack-expect.c
        externals/build/linux64/release/include/mpack/mpack-node.c
        externals/build/linux64/release/include/mpack/mpack-platform.c
        externals/build/linux64/release/include/mpack/mpack-reader.c
        externals/build/linux64/release/include/mpack/mpack-writer.c

        ########################################################################
        # CELIB: Containers
        ########################################################################
        sources/celib/containers/tests/test_array.cc

        ########################################################################
        # CELIB: Memory
        ########################################################################
        sources/celib/memory/private/mallocator.c
        sources/celib/memory/private/allocator.c
        sources/celib/memory/private/memory.c


        ########################################################################
        # CELIB: string
        ########################################################################
        sources/celib/string/private/string.c
        sources/celib/string/tests/test_string.cc


        ########################################################################
        # CELIB: Utils
        ########################################################################
        sources/celib/utils/private/utils.c


        ########################################################################
        # CELIB: Log
        ########################################################################
        sources/celib/log/private/log.c
        sources/celib/log/private/stdout_handler.c
        sources/celib/log/private/nanomsg_handler.c
        sources/celib/log/tests/test_log.cc


        ########################################################################
        # CELIB: Crypto
        ########################################################################
        sources/celib/crypto/private/murmur.c
        sources/celib/crypto/tests/test_murmur.cc
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
set(RELEASE_LIBS
    jemalloc_pic
    pthread
    anl # NANOMSG

    bgfxRelease
    SDL2
    nanomsg
)

################################################################################
# Develop sources
################################################################################
set(DEVELOP_LIBS
        ${RELEASE_LIBS}
        nanomsg
        assimp
        )

################################################################################
# Cetech RUNTIME
################################################################################
add_executable(cetech ${MAIN_SOURCE_FILES})
target_compile_definitions(cetech PUBLIC -DCETECH_RELEASE)
target_link_libraries(cetech ${RELEASE_LIBS} )
target_include_directories(cetech PUBLIC externals/build/linux64/release/ )


################################################################################
# Cetech DEVELOP
################################################################################
add_executable(cetech_develop ${MAIN_SOURCE_FILES})
target_compile_definitions(cetech_develop PUBLIC -DCETECH_DEVELOP)
target_link_libraries(cetech_develop ${DEVELOP_LIBS} )
target_include_directories(cetech_develop PUBLIC externals/build/linux64/release/ )


################################################################################
# Cetech TESTS
################################################################################
add_executable(cetech_tests ${TEST_SOURCE_FILES})
target_compile_definitions(cetech_tests PUBLIC -DCETECH_TEST -DCETECH_DEVELOP)
target_link_libraries(cetech_tests ${DEVELOP_LIBS} )
target_include_directories(cetech_tests PUBLIC externals/build/linux64/release/ )