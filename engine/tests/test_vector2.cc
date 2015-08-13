#include "catch.hpp"

#include "common/memory.h"
#include "common/vector2.h"

SCENARIO( "Basic vector operation", "[vector2]" ) {
    GIVEN( "Two vector" ) {
        cetech1::Vector2 v1 = {1.0f, 2.0f};
        cetech1::Vector2 v2 = {3.0f, 4.0f};

        WHEN( "v1 + v2" ) {
            cetech1::Vector2 v3 = v1 + v2;

            THEN( "final vector is [4.0f, 6.0f]" ) {
                REQUIRE( v3.x == 4.0f );
                REQUIRE( v3.y == 6.0f );
            }
        }

        WHEN( "v1 - v2" ) {
            cetech1::Vector2 v3 = v1 - v2;

            THEN( "final vector is [-2.0f, 2.0f]" ) {
                REQUIRE( v3.x == -2.0f );
                REQUIRE( v3.y == -2.0f );
            }
        }

        WHEN( "v1 * v2" ) {
            cetech1::Vector2 v3 = v1 * v2;

            THEN( "final vector is [3.0f, 8.0f]" ) {
                REQUIRE( v3.x == 3.0f );
                REQUIRE( v3.y == 8.0f );
            }
        }

        WHEN( "v1 / v2" ) {
            cetech1::Vector2 v3 = v1 / v2;

            THEN( "final vector is [0.33333f, 5.0f]" ) {
                REQUIRE( cetech1::math::fcmp(v3.x, 0.33333f) );
                REQUIRE( cetech1::math::fcmp(v3.y, 0.5f) );
            }
        }
    }

    GIVEN( "Vector and scalar" ) {
        cetech1::Vector2 v1 = {1.0f, 2.0f};
        float scalar = 2.0f;

        WHEN( "v1 * scalar" ) {
            cetech1::Vector2 v2 = v1 * scalar;

            THEN( "(v1 * scalar) == [2.0f, 4.0f]" ) {
                REQUIRE( v2.x == 2.0f );
                REQUIRE( v2.y == 4.0f );
            }
        }

        WHEN( "v1 / scalar" ) {
            cetech1::Vector2 v2 = v1 / scalar;

            THEN( "(v1 / scalar) == [0.5f, 1.0f]" ) {
                REQUIRE( v2.x == 0.5f );
                REQUIRE( v2.y == 1.0f );
            }
        }
    }
}

SCENARIO( "Vector len", "[vector2]" ) {
    GIVEN( "Vector [1.0f, 2.0f]" ) {
        cetech1::Vector2 v1 = {1.0f, 2.0f};

        WHEN( "len(v1)" ) {
            float len = cetech1::vector2::len(v1);

            THEN( "Len is 2.25" ) {
                REQUIRE( cetech1::math::fcmp(len, 2.25f));
            }
        }

        WHEN( "len_sq(v1)" ) {
            float len = cetech1::vector2::len_sq(v1);

            THEN( "Len is 5.0f" ) {
                REQUIRE( len == 5.0f );
            }
        }

        WHEN( "len_inv(v1)" ) {
            float len = cetech1::vector2::len_inv(v1);

            THEN( "Len is 5.0f" ) {
                REQUIRE( cetech1::math::fcmp(len, 0.44714f) );
            }
        }

    }
}

SCENARIO( "Normalized", "[vector2]" ) {
    GIVEN( "Vector" ) {
        cetech1::Vector2 v1 = {1.0f, 0.0f};

        WHEN( "normalized(1)" ) {
            cetech1::Vector2 v_norm = cetech1::vector2::normalized(v1);
            const float len = cetech1::vector2::len(v_norm);

            THEN( "len(normalized(1)) == 1.0f" ) {
                REQUIRE( cetech1::math::fcmp(len, 1.0f, 0.01f) ); // TODO: check compare
            }
        }
    }
}

SCENARIO( "Dot product", "[vector2]" ) {
    GIVEN( "Two vector" ) {
        cetech1::Vector2 v1 = {1.0f, 2.0f};
        cetech1::Vector2 v2 = {3.0f, 4.0f};

        WHEN( "v1 . v2" ) {
            float dot = cetech1::vector2::dot(v1, v2);

            THEN( "Dot product is 11.0f" ) {
                REQUIRE( dot == 11.0f );
            }
        }
    }
}
