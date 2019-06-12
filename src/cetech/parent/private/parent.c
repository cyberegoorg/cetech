#include <celib/module.h>
#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/containers/array.h>
#include <celib/api.h>
#include <celib/memory/memory.h>
#include <celib/log.h>
#include <celib/id.h>

#include <cetech/ecs/ecs.h>
#include <cetech/parent/parent.h>

#define _G parent_global
static struct _G {
    ce_alloc_t0 *alloc;
} _G = {};

void _spawn_children(ct_world_t0 world,
                     struct ct_entity_t0 *entities,
                     ct_ecs_ent_chunk_o0 *item,
                     uint32_t n,
                     void *_data) {
    ct_ecs_cmd_buffer_t *cmd = _data;

    ct_parent_c *parent = ct_ecs_c_a0->get_all(world, CT_PARENT_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ct_child_c *child = ct_ecs_c_a0->get_one(world, CT_CHILD_COMPONENT,
                                                 parent[i].parent, true);
        if (!child) {
            ct_ecs_a0->buff_add_component(cmd, world, parent[i].parent,
                                          CE_ARR_ARG(((ct_component_pair_t0[]) {
                                                  {
                                                          .type = CT_CHILD_COMPONENT,
                                                          .data = &(ct_child_c) {
                                                          }
                                                  }
                                          })));
        }
    }
}


void _spawn_previous_parent(ct_world_t0 world,
                            struct ct_entity_t0 *entities,
                            ct_ecs_ent_chunk_o0 *item,
                            uint32_t n,
                            void *_data) {
    ct_ecs_cmd_buffer_t *cmd = _data;

    ct_parent_c *parent = ct_ecs_c_a0->get_all(world, CT_PARENT_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ct_child_c *child = ct_ecs_c_a0->get_one(world, CT_CHILD_COMPONENT,
                                                 parent[i].parent, true);
        if (child) {
            ce_array_push(child->child, entities[i], _G.alloc);
        }

        ct_ecs_a0->buff_add_component(cmd, world, entities[i],
                                      CE_ARR_ARG(((ct_component_pair_t0[]) {
                                              {
                                                      .type = CT_PREVIOUS_PARENT_COMPONENT,
                                                      .data = &(ct_previous_parent_c) {
                                                              .parent = parent[i].parent,
                                                      }
                                              }
                                      })));
    }
}


void _array_swap_remove(ct_entity_t0 *ents,
                        ct_entity_t0 ent) {
    uint32_t ch_n = ce_array_size(ents);
    for (int j = 0; j < ch_n; ++j) {
        if (ents[j].h != ent.h) {
            continue;
        }

        ents[j] = ents[ch_n - 1];
        ce_array_pop_back(ents);
        break;
    }
}

void _destroy_previous_parent2(ct_world_t0 world,
                               struct ct_entity_t0 *entities,
                               ct_ecs_ent_chunk_o0 *item,
                               uint32_t n,
                               void *_data) {
    ct_ecs_cmd_buffer_t *cmd = _data;

    ct_parent_c *parent = ct_ecs_c_a0->get_all(world, CT_PREVIOUS_PARENT_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ct_child_c *child = ct_ecs_c_a0->get_one(world, CT_CHILD_COMPONENT,
                                                 parent[i].parent, true);
        if (child && !ce_array_size(child->child)) {
            ct_ecs_a0->buff_remove_component(cmd, world, parent[i].parent,
                                             (uint64_t[]) {CT_CHILD_COMPONENT}, 1);
        }

        ct_ecs_a0->buff_remove_component(cmd, world, entities[i],
                                         (uint64_t[]) {CT_PREVIOUS_PARENT_COMPONENT}, 1);
    }
}

void _destroy_previous_parent(ct_world_t0 world,
                              struct ct_entity_t0 *entities,
                              ct_ecs_ent_chunk_o0 *item,
                              uint32_t n,
                              void *_data) {
//    ct_ecs_cmd_buffer_t *cmd = _data;

    ct_previous_parent_c *previous_parent = ct_ecs_c_a0->get_all(world,
                                                                 CT_PREVIOUS_PARENT_COMPONENT,
                                                                 item);

    for (int i = 0; i < n; ++i) {
        ct_child_c *child = ct_ecs_c_a0->get_one(world, CT_CHILD_COMPONENT,
                                                 previous_parent[i].parent,
                                                 true);

        if (!child) {
            continue;
        }

        _array_swap_remove(child->child, entities[i]);

//        ct_ecs_a0->buff_remove_component(cmd, world, entities[i],
//                                         (uint64_t[]) {CT_PREVIOUS_PARENT_COMPONENT}, 1);
    }
}

void _change_previous_parent(ct_world_t0 world,
                             struct ct_entity_t0 *entities,
                             ct_ecs_ent_chunk_o0 *item,
                             uint32_t n,
                             void *_data) {
    ct_ecs_cmd_buffer_t *cmd = _data;

    ct_parent_c *parent = ct_ecs_c_a0->get_all(world, CT_PARENT_COMPONENT, item);
    ct_parent_c *prev_parent = ct_ecs_c_a0->get_all(world, CT_PREVIOUS_PARENT_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        if (parent[i].parent.h == prev_parent[i].parent.h) {
            continue;
        }

        ct_child_c *prev_child = ct_ecs_c_a0->get_one(world, CT_CHILD_COMPONENT,
                                                      prev_parent[i].parent, true);
        if (prev_child) {
            _array_swap_remove(prev_child->child, entities[i]);
        }

        ct_child_c *child = ct_ecs_c_a0->get_one(world, CT_CHILD_COMPONENT, parent[i].parent, true);
        if (!child) {
            ct_ecs_a0->buff_add_component(cmd, world, parent[i].parent,
                                          CE_ARR_ARG(((ct_component_pair_t0[]) {
                                                  {
                                                          .type = CT_CHILD_COMPONENT,
                                                          .data = &(ct_child_c) {
                                                          }
                                                  }
                                          })));
        }
    }
}

void _change_previous_parent2(ct_world_t0 world,
                              struct ct_entity_t0 *entities,
                              ct_ecs_ent_chunk_o0 *item,
                              uint32_t n,
                              void *_data) {
    ct_parent_c *parent = ct_ecs_c_a0->get_all(world, CT_PARENT_COMPONENT, item);
    ct_parent_c *prev_parent = ct_ecs_c_a0->get_all(world, CT_PREVIOUS_PARENT_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        if (parent[i].parent.h == prev_parent[i].parent.h) {
            continue;
        }

        ct_child_c *child = ct_ecs_c_a0->get_one(world, CT_CHILD_COMPONENT,
                                                 parent[i].parent, true);
        if (child) {
            ce_array_push(child->child, entities[i], _G.alloc);
        }

        prev_parent[i].parent.h = parent[i].parent.h;
    }
}

static void spawn_children_system(ct_world_t0 world,
                                  float dt,
                                  uint32_t rq_version,
                                  ct_ecs_cmd_buffer_t *cmd) {

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(CT_PARENT_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(CT_PREVIOUS_PARENT_COMPONENT)
                                }, rq_version,
                                _spawn_children, cmd);

}

static void change_previous_parent_system(ct_world_t0 world,
                                          float dt,
                                          uint32_t rq_version,
                                          ct_ecs_cmd_buffer_t *cmd) {
    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(CT_PARENT_COMPONENT,
                                                                CT_PREVIOUS_PARENT_COMPONENT),
                                        .write = CT_ECS_ARCHETYPE(CT_PREVIOUS_PARENT_COMPONENT),
                                        .only_changed = true,

                                }, rq_version,
                                _change_previous_parent, cmd);
}

static void change_previous_parent_system2(ct_world_t0 world,
                                           float dt,
                                           uint32_t rq_version,
                                           ct_ecs_cmd_buffer_t *cmd) {
    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(CT_PARENT_COMPONENT,
                                                                CT_PREVIOUS_PARENT_COMPONENT),
                                        .write = CT_ECS_ARCHETYPE(CT_PREVIOUS_PARENT_COMPONENT),
                                        .only_changed = true,

                                }, rq_version,
                                _change_previous_parent2, cmd);
}

#define SPAWN_CHILDERN \
    CE_ID64_0("spawn_children", 0x968691c5e42491dbULL)

#define CHANGE_PREVIOUS_PARENT \
    CE_ID64_0("chnage_previous_parent", 0x5931b9800e923df8ULL)

#define CHANGE_PREVIOUS2_PARENT \
    CE_ID64_0("chnage_previous_parent2", 0x9189a42e6e009b8ULL)

static struct ct_system_i0 spawn_children_system_i0 = {
        .name = SPAWN_CHILDERN,
        .process = spawn_children_system,
};

static struct ct_system_i0 change_previous_parent_system_i0 = {
        .name = CHANGE_PREVIOUS_PARENT,
        .after = CT_ECS_AFTER(SPAWN_CHILDERN),
        .process = change_previous_parent_system,
};

static struct ct_system_i0 change_previous_parent_system2_i0 = {
        .name = CHANGE_PREVIOUS2_PARENT,
        .after = CT_ECS_AFTER(CHANGE_PREVIOUS_PARENT),
        .process = change_previous_parent_system2,
};

static void parent_system(ct_world_t0 world,
                          float dt,
                          uint32_t rq_version,
                          ct_ecs_cmd_buffer_t *cmd) {


    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(CT_PREVIOUS_PARENT_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(CT_PARENT_COMPONENT)
                                }, rq_version,
                                _destroy_previous_parent, cmd);

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(CT_PREVIOUS_PARENT_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(CT_PARENT_COMPONENT),
                                }, rq_version,
                                _destroy_previous_parent2, cmd);

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(CT_PARENT_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(CT_PREVIOUS_PARENT_COMPONENT)
                                }, rq_version,
                                _spawn_previous_parent, cmd);


}

static struct ct_system_i0 parent_system_i0 = {
        .name = CT_PARENT_SYSTEM,
        .after = CT_ECS_AFTER(CHANGE_PREVIOUS2_PARENT),
        .process = parent_system,
};

// Parent
static struct ct_ecs_component_i0 parent_c_api = {
        .cdb_type = CT_PARENT_COMPONENT,
        .size = sizeof(ct_parent_c),
};

// Previous parent
static struct ct_ecs_component_i0 previous_parent_c_api = {
        .cdb_type = CT_PREVIOUS_PARENT_COMPONENT,
        .size = sizeof(ct_previous_parent_c),
        .is_system_state = true,
};

// child
static struct ct_ecs_component_i0 child_c_api = {
        .cdb_type = CT_CHILD_COMPONENT,
        .size = sizeof(ct_child_c),
};

void CE_MODULE_LOAD(parent)(struct ce_api_a0 *api,
                            int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_log_a0);

    ce_id_a0->id64("parent");

    _G = (struct _G) {
            .alloc = ce_memory_a0->system,
    };


    api->add_impl(CT_ECS_COMPONENT_I,
                  &parent_c_api, sizeof(parent_c_api));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &previous_parent_c_api, sizeof(previous_parent_c_api));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &child_c_api, sizeof(child_c_api));
    api->add_impl(CT_ECS_SYSTEM_I,
                  &spawn_children_system_i0, sizeof(spawn_children_system_i0));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &parent_system_i0, sizeof(parent_system_i0));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &change_previous_parent_system_i0, sizeof(change_previous_parent_system_i0));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &change_previous_parent_system2_i0, sizeof(change_previous_parent_system2_i0));
}

void CE_MODULE_UNLOAD(parent)(struct ce_api_a0 *api,
                              int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
