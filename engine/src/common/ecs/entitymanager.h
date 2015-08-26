#pragma once

#include "entitymanager_types.h"
#include "common/memory/memory_types.h"

#include "common/types.h"
#include "common/handler.h"

#include "common/memory/memory.h"
#include "common/container/queue.h"

namespace cetech1 {   
    namespace entity_manager_internal {
        typename handler::Handler<uint32_t,uint32_t, unsigned char,22,8,1024> entity_handler;
    }

    namespace entity_manager {
        Entity create(EntityManager& em) {
            return {entity_manager_internal::entity_handler.create(em._free_idx, em._generation)};
        }

        void destroy(EntityManager& em, const Entity e) {
            entity_manager_internal::entity_handler.destroy(e.id, em._free_idx, em._generation);
        }

        bool alive(const EntityManager& em, const Entity e) {
            return entity_manager_internal::entity_handler.alive(e.id, em._generation);
        }
    }
}