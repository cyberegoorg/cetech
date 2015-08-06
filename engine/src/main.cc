#include "memory.h"
#include "math_types.h"
#include "types.h"
#include "array.h"
#include "hash.h"
#include "memory.h"
#include "entitymanager.h"

#include <iostream>

using namespace cetech1;

int main(int argc, char** argv) {
    memory_globals::init();

    Allocator& a = memory_globals::default_allocator();
    {

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
    }

    memory_globals::shutdown();
    return 0;
}