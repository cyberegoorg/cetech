#pragma once

#include "types.h"
#include "memory_types.h"
#include "memory.h"
#include "container_types.h"

namespace cetech1 {
    struct Entity {
        uint32_t id;
    };

    struct EntityManager {
        Array < unsigned char > _generation;
        Queue < uint32_t > _free_idx;

        EntityManager() : _generation(memory_globals::default_allocator()), _free_idx(
                              memory_globals::default_allocator()) {};
    };
}