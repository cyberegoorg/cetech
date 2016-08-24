#if defined(CETECH_TEST)

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include <string.h>
#include "celib/memory/memory.h"
#include "celib/string/string.h"
#include "../filesystem.h"
#include "celib/machine/machine.h"
};

SCENARIO( "map root to root_path", "[filesystem]" ) {
    memsys_init(4*1024*1024);

    GIVEN( "Filesystem" ) {
        filesystem_init();

        WHEN("map root 'foo' to path 'foopath'") {
            stringid64_t root = stringid64_from_string("foo");

            filesystem_map_root_dir(root, "foopath");

            THEN("foo => foopath") {
                REQUIRE(str_compare(filesystem_get_root_dir(root), "foopath") == 0);
            }
        }

        filesystem_shutdown();
    }

    memsys_shutdown();
}

SCENARIO( "Can get fullpath of file for root", "[filesystem]" ) {
    memsys_init(4*1024*1024);

    GIVEN( "Filesystem" ) {
        filesystem_init();

        stringid64_t root = stringid64_from_string("foo");
        filesystem_map_root_dir(root, "foopath");

        WHEN("get fullpath for 'foo.bar'") {
            THEN("path is foopath/foo.bar") {
                char fullpath[128] = {0};

                REQUIRE(filesystem_get_fullpath(root, fullpath, 128, "foo.bar") == 1);
                REQUIRE(str_compare(fullpath, "foopath/foo.bar") == 0);
            }
        }

        filesystem_shutdown();
    }

    memsys_shutdown();
}

SCENARIO( "Can open file", "[filesystem]" ) {
    memsys_init(4*1024*1024);

    char* test_dir = strdup(__FILE__);
    strrchr(test_dir, '/')[1] = '\0';

    GIVEN( "Filesystem" ) {
        filesystem_init();

        stringid64_t root = stringid64_from_string("test_dir");
        filesystem_map_root_dir(root, test_dir);


        WHEN("open file test_file.txt") {
            THEN("file is open") {
                struct vio* f = filesystem_open(root, "test_file.txt", VIO_OPEN_READ);

                REQUIRE(f != NULL);

                vio_close(f);
            }
        }

        filesystem_shutdown();
    }

    memsys_shutdown();
}


#endif
