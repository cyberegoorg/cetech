#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "catch/catch.hpp"

#include "celib/memory/memory.h"
#include "celib/command_line/command_line.h"

using namespace cetech;

/*******************************************************************************
**** comand_line can parse comand line =D
*******************************************************************************/
SCENARIO( "comand_line can parse comand line =D", "[command_line]" ) {
    memory_globals::init();

    GIVEN( "args: foo -v --path foo/bar" ) {
        const char* argv[] = {
            "foo",
            "-v",
            "--path",
            "foo/bar"
        };

        const int argc = sizeof(argv) / sizeof(char*);
        command_line::set_args(argc, argv);

        WHEN( "find args --path -p" ) {
            THEN( "has argument" ) {
                REQUIRE( command_line::has_argument("path", 'p'));
            }

            AND_THEN("index is 2") {
                REQUIRE( command_line::find_argument("path", 'p') == 2);
            }

            AND_THEN("parametr is foo/bar") {
                REQUIRE( command_line::get_parameter("path", 'p') == "foo/bar" );
            }
        }

        WHEN( "find args --verbose -v" ) {
            THEN( "has argument" ) {
                REQUIRE( command_line::has_argument("vetbose", 'v'));
            }

            AND_THEN("index is 1") {
                REQUIRE( command_line::find_argument("vetbose", 'v') == 1);
            }
        }

        WHEN( "find args --invalid -i" ) {
            THEN( "no argument" ) {
                REQUIRE( not command_line::has_argument("invalid", 'i'));
            }

            AND_THEN("index is argc") {
                REQUIRE( command_line::find_argument("invalid", 'i') == argc);
            }

            AND_THEN("parametr is nullptr") {
                REQUIRE( command_line::get_parameter("invalid", 'i') == nullptr);
            }
        }
    }

    memory_globals::shutdown();
}
#endif
