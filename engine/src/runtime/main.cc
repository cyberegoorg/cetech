#include "common/memory.h"
#include "common/math_types.h"
#include "common/types.h"
#include "common/array.h"
#include "common/hash.h"
#include "common/entitymanager.h"

#include <iostream>

using namespace cetech1;

void init() {
    memory_globals::init();
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