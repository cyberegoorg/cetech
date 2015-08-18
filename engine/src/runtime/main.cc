#include "common/memory/memory.h"
#include "common/math/math_types.h"
#include "common/math/math.h"
#include "common/math/quat.h"
#include "common/math/vector2.h"
#include "common/math/matrix33.h"
#include "common/math/simd/simd.h"
#include "common/types.h"
#include "common/container/array.h"
#include "common/container/hash.h"
#include "common/ecs/entitymanager.h"

#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace cetech1;
using namespace rapidjson;



void init() {
    memory_globals::init();

    const char* json = "{\"project\":\"rapidjson\",\"stars\":10}";
    Document d;
    d.Parse(json);

    // 2. Modify it by DOM.
    Value& s = d["stars"];
    s.SetInt(s.GetInt() + 1);

    // 3. Stringify the DOM
    StringBuffer buffer;
    Writer < StringBuffer > writer(buffer);
    d.Accept(writer);

    // Output {"project":"rapidjson","stars":11}
    std::cout << buffer.GetString() << std::endl;

    const cetech1::Vector2 v = cetech1::vector2::normalized({2.0f, 0.0f});

    std::cout << "len: " << cetech1::vector2::len(v) << ", " << cetech1::vector2::is_normalized(v) << std::endl;

    cetech1::Matrix33 m1 = cetech1::matrix33::IDENTITY;
    cetech1::Matrix33 m2 = cetech1::matrix33::IDENTITY;

    cetech1::Matrix33 m3 = m1 * m2;

    printf("sqrt: %f, ivn_sqrt: %f\n", cetech1::math::fast_sqrt(4), cetech1::math::fast_inv_sqrt(4));
}

void run() {
    Allocator& a = memory_globals::default_allocator();

    EntityManager em;

    Array < int > array1(a);
    array::push_back < int > (array1, 1);
    array::push_back < int > (array1, 2);
    array::push_back < int > (array1, 3);

    for (auto it = array::begin(array1); it != array::end(array1); ++it) {
        std::cout << *it << std::endl;
    }

    Entity e1 = entity_manager::create(em);
    printf("[idx: %u, gen: %u]\n", entity::idx(e1), entity::gen(e1));

    entity_manager::destroy(em, e1);

    Entity e2 = entity_manager::create(em);
    printf("[idx: %u, gen: %u]\n", entity::idx(e2), entity::gen(e2));

    Entity e3 = entity_manager::create(em);
    printf("[idx: %u, gen: %u]\n", entity::idx(e3), entity::gen(e3));

    float sin1, cos1;
    sin1 = cos1 = 0;
    cetech1::math::fast_sincos(21, sin1, cos1);

    cetech1::memory::malloc(0);

    printf("%f, %f == %f, %f\n", sin(cetech1::math::deg2rad(21)), cos(cetech1::math::deg2rad(21)), sin1, cos1);
}

void shutdown() {
    memory_globals::shutdown();
}

int main(int argc, char** argv) {
    init();
    run();
    shutdown();

    return 0; // TODO: error check
}
