#pragma once

#include "celib/types.h"
#include "celib/memory/memory.h"
#include "celib/container/container_types.h"

namespace cetech {
    struct Entity {
        uint32_t id;
    };

    struct EntityManager {
        Array < unsigned char > _generation;
        Queue < uint32_t > _free_idx;

        EntityManager() : _generation(memory_globals::default_allocator()),
                          _free_idx(memory_globals::default_allocator()) {};
    };
}
