#ifdef CETECH_TEST

#include "catch/catch.hpp"

#include "celib/memory/memory.h"
#include "celib/command_line/command_line.h"

using namespace cetech;

TEST_CASE( "Parse command line arguments", "[command_line]" ) {
    memory_globals::init();

    const char* argv[] = {
        "foo",
        "-v",
        "--path",
        "foo/bar"
    };

    const int argc = sizeof(argv)/sizeof(char*);
    command_line::set_args(argc, argv);

    REQUIRE( command_line::has_argument("path", 'p') );
    REQUIRE( command_line::has_argument("verbose", 'v') );
    
    REQUIRE( command_line::find_argument("path", 'p') == 2);
    REQUIRE( command_line::find_argument("verbose", 'v') == 1 );
    
    REQUIRE( command_line::get_parameter("path", 'p') == "foo/bar" );
    
    memory_globals::shutdown();
}
#endif