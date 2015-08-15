#pragma once

#define REQUIRE_ALMOST(a, b) REQUIRE( a == Approx(b))
#define REQUIRE_ALMOST3(a, b, e) REQUIRE( a == Approx(b).epsilon(e))