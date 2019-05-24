#include <celib/memory/allocator.h>
#include "celib/config.h"
#include "cetech/resource/resource.h"
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include <celib/ydb.h>
#include <celib/ydb.h>
#include <celib/macros.h>
#include <celib/containers/array.h>
#include <celib/math/math.h>

#include <celib/log.h>
#include <celib/cdb.h>

#include "celib/module.h"
#include "cetech/ecs/ecs.h"
#include <cetech/transform/transform.h>
#include <cetech/debugui/icons_font_awesome.h>

#include <cetech/editor/editor_ui.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/ecs/ecs.h>
#include <celib/containers/hash.h>
#include <sys/mman.h>
#include <cetech/renderer/renderer.h>
#include <cetech/kernel/kernel.h>
#include <cetech/game/game_system.h>
#include <celib/containers/spsc.h>

#define LOG_WHERE "transform"

#define MAX_NODES 1000000
typedef struct world_state_t {
    ce_hash_t component_map;
    ct_world_t0 ent_world;
    ct_entity_t0 *entity;

    uint32_t *parent;
    uint32_t *first_child;
    uint32_t *next_sibling;
    uint32_t *prev_sibling;

    ce_mat4_t *local;
    ce_mat4_t *world;

    uint32_t nodes_num;
    ct_ecs_ev_queue_o0 *events;
} world_state_t;

static struct transform_global {
    ce_hash_t world_map;
    world_state_t *world_state;
    ct_cdb_ev_queue_o0 *changed_obj_queue;
    ce_alloc_t0 *alloc;
} _G = {};


static void *virtual_alloc(uint64_t size) {
    return CE_REALLOC(ce_memory_a0->virt_system, void, NULL, size, 0);
}

static world_state_t *_get_or_create_world_state(ct_world_t0 world) {
    uint64_t idx = ce_hash_lookup(&_G.world_map, world.h, UINT64_MAX);
    if (idx == UINT64_MAX) {
        idx = ce_array_size(_G.world_state);

        ce_array_push(_G.world_state, ((world_state_t) {
                .entity = virtual_alloc(MAX_NODES * sizeof(ct_entity_t0)),
                .parent = virtual_alloc(MAX_NODES * sizeof(uint32_t)),
                .first_child = virtual_alloc(MAX_NODES * sizeof(uint32_t)),
                .next_sibling = virtual_alloc(MAX_NODES * sizeof(uint32_t)),
                .prev_sibling = virtual_alloc(MAX_NODES * sizeof(uint32_t)),
                .world = virtual_alloc(MAX_NODES * sizeof(ce_mat4_t)),
                .local = virtual_alloc(MAX_NODES * sizeof(ce_mat4_t)),
                .ent_world = world,
        }), _G.alloc);
        ce_hash_add(&_G.world_map, world.h, idx, _G.alloc);
    }

    return &_G.world_state[idx];
}

static void _transform(world_state_t *state,
                       uint32_t node_idx) {
    ct_entity_t0 ent = state->entity[node_idx];

    ct_transform_comp *tc = ct_ecs_a0->get_one(state->ent_world, TRANSFORM_COMPONENT, ent);
    if (!tc) {
        return;
    }

    ce_vec3_t rot_rad = ce_vec3_mul_s(tc->rot, CE_DEG_TO_RAD);

    float *local = state->local[node_idx].m;
    ce_mat4_srt(local,
                tc->scl.x, tc->scl.y, tc->scl.z,
                rot_rad.x, rot_rad.y, rot_rad.z,
                tc->pos.x, tc->pos.y, tc->pos.z);

    float *world = state->world[node_idx].m;

    uint32_t parent_idx = state->parent[node_idx];

    if (parent_idx != UINT32_MAX) {
        ce_mat4_mul(world, local, state->world[parent_idx].m);
    } else {
        ce_mat4_move(world, local);
    }

    ce_mat4_move(tc->world.m, world);

    uint32_t it = state->first_child[node_idx];
    while (it != UINT32_MAX) {
        _transform(state, it);
        it = state->next_sibling[it];
    }
}

static uint32_t _get_node(world_state_t *state,
                          ct_entity_t0 entity) {
    return ce_hash_lookup(&state->component_map, entity.h, UINT32_MAX);
}

static uint32_t _create_node(world_state_t *state,
                             ct_entity_t0 entity) {
    uint32_t idx = state->nodes_num++;

    state->entity[idx] = entity;

    state->parent[idx] = UINT32_MAX;
    state->first_child[idx] = UINT32_MAX;
    state->next_sibling[idx] = UINT32_MAX;
    state->prev_sibling[idx] = UINT32_MAX;

    ce_hash_add(&state->component_map, entity.h, idx, _G.alloc);

    _transform(state, idx);

    return idx;
}

static void _link(world_state_t *state,
                  uint32_t child,
                  uint32_t parent) {

    if (UINT32_MAX == child) {
        return;
    }

    if (UINT32_MAX == parent) {
        return;
    }

    if (state->parent[child] == parent) {
        return;
    }

    state->parent[child] = parent;

    uint32_t tmp = state->first_child[parent];

    state->first_child[parent] = child;
    state->next_sibling[child] = tmp;
    state->prev_sibling[child] = UINT32_MAX;

    if (tmp != UINT32_MAX) {
        state->prev_sibling[tmp] = child;
    }

    _transform(state, child);
}

void _unlink(world_state_t *state,
             uint32_t child) {
    if (UINT32_MAX == child) {
        return;
    }

    uint32_t parent = state->parent[child];

    if (UINT32_MAX == parent) {
        return;
    }

    uint32_t prev = state->prev_sibling[child];
    uint32_t next = state->next_sibling[child];

    state->prev_sibling[child] = UINT32_MAX;
    state->next_sibling[child] = UINT32_MAX;

    // first in root
    if (prev == UINT32_MAX) {
        state->first_child[parent] = next;
        state->prev_sibling[next] = UINT32_MAX;
    } else if (next != UINT32_MAX) {
        state->next_sibling[prev] = next;
        state->prev_sibling[next] = prev;
    }

    _transform(state, child);
}

static uint64_t cdb_type() {
    return TRANSFORM_COMPONENT;
}

static const char *display_name() {
    return ICON_FA_ARROWS " Transform";
}

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
        };

        return &ct_editor_component_i0;
    }

    return NULL;
}


static uint64_t _tranform_size() {
    return sizeof(ct_transform_comp);
}

static void _tranform_on_spawn(uint64_t obj,
                               void *data) {
    ct_transform_comp *t = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, t, sizeof(ct_transform_comp));
}

static struct ct_component_i0 ct_component_api = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .size = _tranform_size,
        .on_spawn = _tranform_on_spawn,
        .on_change = _tranform_on_spawn,
};

static void transform_system(ct_world_t0 world,
                             float dt) {
    world_state_t *state = _get_or_create_world_state(world);

    ct_ecs_world_event_t0 ev = {};
    while (ct_ecs_a0->pop_world_events(state->events, &ev)) {
        if (ev.type == CT_ECS_EVENT_COMPONENT_SPAWN) {
            uint64_t component_type = ev.component.type;

            if (component_type != TRANSFORM_COMPONENT) {
                continue;
            }

            uint32_t idx = _create_node(state, ev.component.ent);

            ct_entity_t0 parent = ct_ecs_a0->parent(state->ent_world, ev.component.ent);

            if (parent.h) {
                uint32_t parent_node = _get_node(state, parent);
                _link(state, idx, parent_node);
            }

        } else if (ev.type == CT_ECS_EVENT_COMPONENT_CHANGE) {
            if (ev.component.type != TRANSFORM_COMPONENT) {
                continue;
            }


            uint64_t idx = ce_hash_lookup(&state->component_map,
                                          ev.component.ent.h,
                                          UINT64_MAX);

            if (idx == UINT64_MAX) {
                continue;
            }

            _transform(state, idx);
        } else if (ev.type == CT_ECS_EVENT_ENT_LINK) {
            uint32_t parent_node = _get_node(state, ev.link.parent);
            uint32_t child_node = _get_node(state, ev.link.child);

            if (child_node == UINT32_MAX) {
                continue;
            }

            _link(state, child_node, parent_node);

        } else if (ev.type == CT_ECS_EVENT_ENT_UNLINK) {
            uint32_t child_node = _get_node(state, ev.link.child);

            _unlink(state, child_node);
        }
    }
}

static uint64_t name() {
    return TRANSFORM_SYSTEM;
}

void _on_create_world(ct_world_t0 world,
                      ct_ecs_ev_queue_o0 *q) {
    world_state_t *state = _get_or_create_world_state(world);
    state->events = q;
}

static struct ct_system_i0 transform_system_i0 = {
        .simulation = transform_system,
        .name = name,
        .on_create_world= _on_create_world,
};

static const ce_cdb_prop_def_t0 transform_component_prop[] = {
        {
                .name = "position",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = PROP_POSITION,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
        {
                .name = "rotation",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = PROP_ROTATION,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
        {
                .name = "scale",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = PROP_SCALE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static const ce_cdb_prop_def_t0 position_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT},
        {.name = "z", .type = CE_CDB_TYPE_FLOAT},
};

static const ce_cdb_prop_def_t0 rotation_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT},
        {.name = "z", .type = CE_CDB_TYPE_FLOAT},
};

static const ce_cdb_prop_def_t0 scale_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
        {.name = "z", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
};

static const ce_cdb_prop_def_t0 vec2_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT},
};


static uint64_t _transform_cdb_type() {
    return TRANSFORM_COMPONENT;
}

static void _transform_property_editor(uint64_t obj,
                                       uint64_t context) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);


    uint64_t pos = ce_cdb_a0->read_subobject(r, PROP_POSITION, 0);
    ct_editor_ui_a0->prop_vec3(pos,
                               "Position",
                               (uint64_t[3]) {PROP_POSITION_X, PROP_POSITION_Y, PROP_POSITION_Z},
                               (ui_vec3_p0) {});

    uint64_t rot = ce_cdb_a0->read_subobject(r, PROP_ROTATION, 0);
    ct_editor_ui_a0->prop_vec3(rot,
                               "Rotation",
                               (uint64_t[3]) {PROP_ROTATION_X, PROP_ROTATION_Y, PROP_ROTATION_Z},
                               (ui_vec3_p0) {});

    uint64_t scl = ce_cdb_a0->read_subobject(r, PROP_SCALE, 0);
    ct_editor_ui_a0->prop_vec3(scl,
                               "Scale",
                               (uint64_t[3]) {PROP_SCALE_X, PROP_SCALE_Y, PROP_SCALE_Z},
                               (ui_vec3_p0) {});
}


static struct ct_property_editor_i0 position_property_editor_api = {
        .cdb_type = _transform_cdb_type,
        .draw_ui = _transform_property_editor,
};


void CE_MODULE_LOAD(transform)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_log_a0);

    _G = (struct transform_global) {
            .alloc = ce_memory_a0->system,
            .changed_obj_queue = ce_cdb_a0->new_changed_obj_listener(ce_cdb_a0->db()),
    };

    api->add_impl(CT_ECS_COMPONENT_I,
                  &ct_component_api, sizeof(ct_component_api));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &transform_system_i0, sizeof(transform_system_i0));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &position_property_editor_api, sizeof(position_property_editor_api));

    ce_cdb_a0->reg_obj_type(PROP_POSITION,
                            position_prop, CE_ARRAY_LEN(position_prop));

    ce_cdb_a0->reg_obj_type(PROP_ROTATION,
                            rotation_prop, CE_ARRAY_LEN(rotation_prop));

    ce_cdb_a0->reg_obj_type(PROP_SCALE,
                            scale_prop, CE_ARRAY_LEN(scale_prop));

    ce_cdb_a0->reg_obj_type(VEC2_CDB_TYPE,
                            vec2_prop, CE_ARRAY_LEN(vec2_prop));

    ce_cdb_a0->reg_obj_type(TRANSFORM_COMPONENT,
                            transform_component_prop, CE_ARRAY_LEN(transform_component_prop));

}

void CE_MODULE_UNLOAD(transform)(struct ce_api_a0 *api,
                                 int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
