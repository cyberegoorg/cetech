#pragma once

#include "entitymanager_types.h"
#include "memory_types.h"

#include "types.h"
#include "memory.h"
#include "queue.h"

#if !defined(ENTITY_INDEX_BIT_COUNT)
  #define ENTITY_INDEX_BIT_COUNT 22
#endif

#if !defined(ENTITY_GENERATION_BIT_COUNT)
  #define ENTITY_GENERATION_BIT_COUNT 8
#endif

/* TODO: compile-time 2^ENTITY_INDEX_BIT_COUNT */
#if !defined(MAX_ENTITIES)
  #define MAX_ENTITIES 4194304
#endif

#if !defined(MAX_ENTITY_NAME_LEN)
  #define MAX_ENTITY_NAME_LEN 64
#endif

#define MINIMUM_FREE_INDEXS 1024
#define FREE_INDEX_COUNT (_entity_system.free_indices_last + 1)
#define GENERATIONS_COUNT (_entity_system.generation_last + 1)

#define ENTITY_INDEX_MASK ((1 << ENTITY_INDEX_BIT_COUNT) - 1)
#define ENTITY_GENERATION_MASK ((1 << ENTITY_GENERATION_BIT_COUNT) - 1)

namespace cetech1 {

    namespace entity {
        uint32_t idx(const Entity& ent) {
            return ent.id >> ENTITY_INDEX_BIT_COUNT;
        }

        uint32_t gen(const Entity& ent) {
            return ent.id & ENTITY_GENERATION_MASK;
        }

        inline Entity make_entity(uint32_t idx, uint32_t gen) {
            Entity ent;
            ent.id = (((idx) << (ENTITY_INDEX_BIT_COUNT)) | (gen));
            return ent;
        }
    }

    namespace entity_manager {
        Entity create(EntityManager& em) {
            uint32_t idx;

            if (queue::size(em._free_idx) > MINIMUM_FREE_INDEXS) {
                idx = em._free_idx[0];
                queue::pop_front(em._free_idx);
            } else {
                array::push_back < unsigned char > (em._generation, 0);
                idx = array::size(em._generation) - 1;
            }

            return entity::make_entity(idx, em._generation[idx]);
        }

        void destroy(EntityManager& em, Entity e) {
            const auto idx = entity::idx(e);

            ++em._generation[idx];
            queue::push_back(em._free_idx, idx);
        }

        bool alive(const EntityManager& em, Entity e) {
            return em._generation[entity::idx(e)] == entity::gen(e);
        }
    }
}