#ifdef CETECH_TEST

#include "include/catch/catch.hpp"

extern "C" {
#include <string.h>
#include "celib/memory/memory.h"
#include "../file.h"
#include "celib/string/string.h"
};

SCENARIO( "Can read file", "[vfile]" ) {
    memsys_init(4*1024*1024);

    char* test_dir = strdup(__FILE__);
    strrchr(test_dir, '/')[1] = '\0';

    GIVEN( "test_file.txt" ) {
        char path[1024] = {0};
        strcat(strcat(path, test_dir), "test_file.txt");

        struct vfile* f = vfile_from_file(path, VFILE_OPEN_READ, memsys_main_allocator());
        REQUIRE( f != NULL);

        int64_t f_size = vfile_size(f);
        REQUIRE( f_size > 0 );

        char buffer[f_size+1];
        vfile_read(f, buffer, sizeof(char), f_size);
        buffer[f_size] = '\0';

        REQUIRE( str_compare(buffer, "test data\n") == 0);

        vfile_close(f);
    }

    memsys_shutdown();
}

#endif
