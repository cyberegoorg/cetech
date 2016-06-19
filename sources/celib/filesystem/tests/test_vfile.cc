#ifdef CETECH_TEST

#include <string.h>

#include "include/catch/catch.hpp"

extern "C" {
#include "../vfile.h"
#include "../../string/string.h"
};

SCENARIO( "Can read file", "[vfile]" ) {
    char* test_dir = strdup(__FILE__);
    strrchr(test_dir, '/')[1] = '\0';

    GIVEN( "test_file.txt" ) {
        char path[1024] = {0};
        strcat(strcat(path, test_dir), "test_file.txt");

        vfile_t f = vfile_from_file(path, VFILE_READ);
        REQUIRE( f != NULL);

        size_t f_size = vfile_size(f);
        REQUIRE( f_size > 0 );

        char buffer[f_size+1];
        vfile_read(f, buffer, f_size);
        buffer[f_size] = '\0';

        REQUIRE( str_compare(buffer, "test data\n") == 0);
    }
}

#endif
