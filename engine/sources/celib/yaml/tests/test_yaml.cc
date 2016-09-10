#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "../yaml.h"
};

/*******************************************************************************
**** Test helpers
*******************************************************************************/


/*******************************************************************************
**** Yaml parse
*******************************************************************************/
SCENARIO("Yaml string parse", "[yaml]") {
    GIVEN("Yaml in string") {

        static const char *yaml_str = \
            "map:\n"
                "  seq: [1, 2, 3]\n"
                "  int: 1\n"
                "  float: 3.14\n"
                "  bool: true\n";

        WHEN("Parse string") {
            yaml_document_t h;
            yaml_node_t root = yaml_load_str(yaml_str, &h);

            THEN("h.d != NULL") {
                REQUIRE(h.d != NULL);

            }
            AND_THEN("check map") {

                YAML_NODE_SCOPE(tmp_node,  root, "map", {
                    REQUIRE(yaml_node_type( tmp_node) == YAML_TYPE_MAP);
                });

            }
            AND_THEN("check childs") {
                YAML_NODE_SCOPE(map_node,  root, "map", {

                    YAML_NODE_SCOPE(seq_node,  map_node, "seq", {
                            REQUIRE(yaml_node_type( seq_node) == YAML_TYPE_SEQ);

                            YAML_NODE_SCOPE_IDX(tmp_node, seq_node, 0, {
                                REQUIRE(yaml_node_type( tmp_node) == YAML_TYPE_SCALAR);
                                REQUIRE(yaml_node_as_int( tmp_node) == 1);
                            });

                            YAML_NODE_SCOPE_IDX(tmp_node,  seq_node, 1, {
                                REQUIRE(yaml_node_type( tmp_node) == YAML_TYPE_SCALAR);
                                REQUIRE(yaml_node_as_int( tmp_node) == 2);
                            });

                            YAML_NODE_SCOPE_IDX(tmp_node,  seq_node, 2, {
                                REQUIRE(yaml_node_type( tmp_node) == YAML_TYPE_SCALAR);
                                REQUIRE(yaml_node_as_int( tmp_node) == 3);
                            });

                    });

                    YAML_NODE_SCOPE(tmp_node,  map_node, "int", {
                            REQUIRE(yaml_node_type( tmp_node) == YAML_TYPE_SCALAR);
                            REQUIRE(yaml_node_as_int( tmp_node) == 1);
                    });

                    YAML_NODE_SCOPE(tmp_node,  map_node, "bool", {
                            REQUIRE(yaml_node_type( tmp_node) == YAML_TYPE_SCALAR);
                            REQUIRE(yaml_node_as_bool( tmp_node));
                    });

                    YAML_NODE_SCOPE(tmp_node,  map_node, "float", {
                            REQUIRE(yaml_node_type( tmp_node) == YAML_TYPE_SCALAR);
                            REQUIRE(yaml_node_as_float( tmp_node));
                    });

                });

            }
        }
    }
}

#endif
