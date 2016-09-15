#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/types.h"
#include "celib/handler/handler_manager.h"
#include "celib/memory/memory.h"
#include "celib/container/queue.inl.h"
#include "celib/ecs/types.h"
#include "celib/memory/types.h"

namespace cetech {

    /***************************************************************************
    **** Entity manager interface
    ***************************************************************************/
    namespace entity_manager {
        CE_INLINE Entity create(EntityManager& em);
        CE_INLINE void destroy(EntityManager& em,
                               const Entity e);

        CE_INLINE bool alive(const EntityManager& em,
                             const Entity e);
    }

    /***************************************************************************
    **** Entity manager internals
    ***************************************************************************/
    namespace entity_manager_internal {
        typename handler::HandlerManager < uint32_t, uint32_t, unsigned char, 22, 8, 1024 > entity_handler_manager;
    }

    /***************************************************************************
    **** Entity manager implementations
    ***************************************************************************/
    namespace entity_manager {
        Entity create(EntityManager& em) {
            return {
                       entity_manager_internal::entity_handler_manager.create(em._free_idx, em._generation)
            };
        }

        void destroy(EntityManager& em,
                     const Entity e) {
            entity_manager_internal::entity_handler_manager.destroy(e.id, em._free_idx, em._generation);
        }

        bool alive(const EntityManager& em,
                   const Entity e) {
            return entity_manager_internal::entity_handler_manager.alive(e.id, em._generation);
        }
    }
}
