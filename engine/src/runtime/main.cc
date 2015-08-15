#include "common/memory.h"
#include "common/math_types.h"
#include "common/math.h"
#include "common/types.h"
#include "common/array.h"
#include "common/hash.h"
#include "common/entitymanager.h"

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
