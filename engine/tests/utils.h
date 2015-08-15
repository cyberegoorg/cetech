#pragma once

#define REQUIRE_ALMOST(a, b) REQUIRE(cetech1::math::almost_equal((a), (b)))
#define REQUIRE_ALMOST3(a, b, e) REQUIRE(cetech1::math::almost_equal((a), (b), (e)))