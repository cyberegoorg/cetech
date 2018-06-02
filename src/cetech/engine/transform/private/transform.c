#include "cetech/kernel/config/config.h"
#include "cetech/engine/resource/resource.h"
#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"


#include "cetech/engine/ecs/ecs.h"
#include <cetech/engine/transform/transform.h>
#include <cetech/kernel/yaml/ydb.h>
#include <cetech/macros.h>
#include <cetech/kernel/containers/array.h>
#include <cetech/kernel/math/fmath.h>
#include <cetech/kernel/ebus/ebus.h>

#include "cetech/kernel/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_ebus_a0);

struct WorldInstance {
    struct ct_world world;
    uint32_t n;
    uint32_t allocated;
    void *buffer;

    struct ct_entity *entity;

    uint32_t *first_child;
    uint32_t *next_sibling;
    uint32_t *parent;

    float *position;
    float *rotation;
    float *scale;
    float *world_matrix;
};


#define _G TransformGlobal
static struct _G {
    uint64_t type;

//    struct ct_hash_t world_map;
//    struct WorldInstance *world_instances;
//    struct ct_hash_t ent_map;
    struct ct_alloc *allocator;
} _G;


//static uint64_t hash_combine(uint32_t a,
//                             uint32_t b) {
//    union {
//        struct {
//            uint32_t a;
//            uint32_t b;
//        };
//        uint64_t ab;
//    } c{
//            .a = a,
//            .b = b,
//    };
//
//    return c.ab;
//}

//#define hash_combine(a, b) (((a) * 11)^(b))


//static void allocate(WorldInstance &_data,
//                     ct_alloc *_allocator,
//                     uint32_t sz) {
//    //assert(sz > _data.n);
//
//    WorldInstance new_data;
//    const unsigned bytes = sz * (sizeof(ct_entity) +
//                                 (3 * sizeof(uint32_t)) +
//                                 (2 * sizeof(float) * 3) +
//                                 (sizeof(float) * 4) +
//                                 (sizeof(float) * 16));
//    new_data.buffer = CT_ALLOC(_allocator, char, bytes);
//    new_data.n = _data.n;
//    new_data.allocated = sz;
//
//    new_data.entity = (ct_entity *) (new_data.buffer);
//    new_data.first_child = (uint32_t *) (new_data.entity + sz);
//    new_data.next_sibling = (uint32_t *) (new_data.first_child + sz);
//    new_data.parent = (uint32_t *) (new_data.next_sibling + sz);
//    new_data.position = (float *) (new_data.parent + sz);
//    new_data.rotation = (float *) (new_data.position + (sz * 3));
//    new_data.scale = (float *) (new_data.rotation + (sz * 4));
//    new_data.world_matrix = (float *) (new_data.scale + (sz * 3));
//
//    memcpy(new_data.entity, _data.entity, _data.n * sizeof(ct_entity));
//
//    memcpy(new_data.first_child, _data.first_child, _data.n * sizeof(uint32_t));
//    memcpy(new_data.next_sibling, _data.next_sibling,
//           _data.n * sizeof(uint32_t));
//    memcpy(new_data.parent, _data.parent, _data.n * sizeof(uint32_t));
//
//    memcpy(new_data.position, _data.position, _data.n * sizeof(float) * 3);
//    memcpy(new_data.rotation, _data.rotation, _data.n * sizeof(float) * 4);
//    memcpy(new_data.scale, _data.scale, _data.n * sizeof(float) * 3);
//
//    memcpy(new_data.world_matrix, _data.world_matrix,
//           _data.n * sizeof(float) * 16);
//
//    CT_FREE(_allocator, _data.buffer);
//
//    _data = new_data;
//}

//static void _new_world(struct ct_world world) {
//    uint32_t idx = ct_array_size(_G.world_instances);
//    ct_array_push(_G.world_instances, WorldInstance(), _G.allocator);
//    _G.world_instances[idx].world = world;
//    ct_hash_add(&_G.world_map, world.h, idx, _G.allocator);
//}


//static WorldInstance *_get_world_instance(ct_world world) {
//    uint32_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT32_MAX);
//
//    if (idx != UINT32_MAX) {
//        return &_G.world_instances[idx];
//    }
//
//    return nullptr;
//}

//static void _destroy_world(struct ct_world world) {
//    uint32_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT32_MAX);
//    uint32_t last_idx = ct_array_size(_G.world_instances) - 1;
//
//    struct ct_world last_world = _G.world_instances[last_idx].world;
//
//    CT_FREE(ct_memory_a0.main_allocator(),
//            _G.world_instances[idx].buffer);
//
//    _G.world_instances[idx] = _G.world_instances[last_idx];
//    ct_hash_add(&_G.world_map, last_world.h, idx, _G.allocator);
//    ct_array_pop_back(_G.world_instances);
//}

void _component_compiler(uint32_t ebus,
                         void *event) {
    struct ct_ecs_component_compile_ev *ev = event;

    struct ct_transform_comp t_data;

    struct ct_yng_doc *d = ct_ydb_a0.get(ev->filename);
    uint64_t keys[ev->component_key_count + 1];
    memcpy(keys, ev->component_key, sizeof(uint64_t) * ev->component_key_count);

    uint64_t key;

    keys[ev->component_key_count] = ct_yng_a0.key("scale");
    key = ct_yng_a0.combine_key(keys, CT_ARRAY_LEN(keys));
    if (d->has_key(d, key)) {
        ct_ydb_a0.get_vec3(ev->filename, keys, CT_ARRAY_LEN(keys),
                           t_data.scale, (float[3]) {0});
        ct_cdb_a0.set_vec3(ev->writer, PROP_SCALE, t_data.scale);
    }


    keys[ev->component_key_count] = ct_yng_a0.key("position");
    key = ct_yng_a0.combine_key(keys, CT_ARRAY_LEN(keys));
    if (d->has_key(d, key)) {
        ct_ydb_a0.get_vec3(ev->filename, keys, CT_ARRAY_LEN(keys),
                           t_data.position, (float[3]) {0});
        ct_cdb_a0.set_vec3(ev->writer, PROP_POSITION, t_data.position);
    }

    keys[ev->component_key_count] = ct_yng_a0.key("rotation");
    key = ct_yng_a0.combine_key(keys, CT_ARRAY_LEN(keys));
    if (d->has_key(d, key)) {
        ct_ydb_a0.get_vec3(ev->filename, keys, CT_ARRAY_LEN(keys),
                           t_data.rotation, (float[3]) {0});
        ct_cdb_a0.set_vec3(ev->writer, PROP_ROTATION, t_data.rotation);
    }
}

void transform_transform(struct ct_transform_comp *transform,
                         float *parent) {

    float *pos = transform->position;
    float *rot = transform->rotation;
    float *sca = transform->scale;

    float tm[16];
    float rm[16];
    float trm[16];
//    float sm[16];

    float rot_rad[3];
    ct_vec3_mul_s(rot_rad, rot, CT_DEG_TO_RAD);

    ct_mat4_translate(tm, pos[0], pos[1], pos[2]);
//    ct_mat4_scale(sm, sca[0], sca[1], sca[2]);
//    ct_mat4_rotate_xyz(sm, rot_rad[0], rot_rad[1], rot_rad[2]);

//    ct_mat4_srt(m,
//                sca[0], sca[1], sca[2],
//                rot_rad[0], rot_rad[1], rot_rad[2],
//                pos[0], pos[1], pos[2]);
    ct_mat4_mul(trm, tm, rm);
    ct_mat4_mul(transform->world, trm, sca);
    ct_mat4_move(transform->world, tm);
}


static void on_change(uint32_t bus_name,
                      void *event) {
    struct ct_ecs_component_ev *ev = event;

    if (!(ev->comp_mask & ct_ecs_a0.component_mask(_G.type))) {
        return;
    }

    struct ct_transform_comp *transform;
    transform = ct_ecs_a0.entity_data(ev->world, TRANSFORM_COMPONENT,
                                      ev->ent);

    transform_transform(transform, NULL);
}


static struct ct_transform_a0 _api = {
};

static void _component_spawner(uint32_t ebus,
                               void *event) {
    struct ct_ecs_component_spawn_ev *ev = event;

    struct ct_transform_comp *transform = (struct ct_transform_comp *) (ev->data);

    ct_cdb_a0.read_vec3(ev->obj, PROP_POSITION, transform->position);
    ct_cdb_a0.read_vec3(ev->obj, PROP_ROTATION, transform->rotation);
    ct_cdb_a0.read_vec3(ev->obj, PROP_SCALE, transform->scale);

    transform_transform(transform, NULL);
}

static void _init(struct ct_api_a0 *api) {
    api->register_api("ct_transform_a0", &_api);


    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("transform"),
    };


    static struct ct_component_prop_map prop_map[] = {
            {
                    .ui_name = "POSITION",
                    .key = "position",
                    .offset = offsetof(struct ct_transform_comp, position),
                    .type = CDB_TYPE_VEC3,

            },
            {
                    .ui_name = "ROTATION",
                    .key = "rotation",
                    .offset = offsetof(struct ct_transform_comp, rotation),
                    .type = CDB_TYPE_VEC3,
                    .limit = {.min_f = 0, .max_f = 360}
            },
            {
                    .ui_name = "SCALE",
                    .key = "scale",
                    .offset = offsetof(struct ct_transform_comp, scale),
                    .type = CDB_TYPE_VEC3,
            }
    };

    ct_ecs_a0.register_component(
            (struct ct_component_info) {
                    .size = sizeof(struct ct_transform_comp),
                    .component_name = TRANSFORMATION_COMPONENT_NAME,
                    .prop_map = prop_map,
                    .prop_count = CT_ARRAY_LEN(prop_map)});

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                            CT_ID64_0(TRANSFORMATION_COMPONENT_NAME),
                            _component_spawner, 0);

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_COMPILE,
                            CT_ID64_0(TRANSFORMATION_COMPONENT_NAME),
                            _component_compiler, 0);

    ct_ebus_a0.connect(ECS_EBUS, ECS_COMPONENT_CHANGE, on_change, 0);

}

static void _shutdown() {
    ct_ebus_a0.disconnect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                               CT_ID64_0(TRANSFORMATION_COMPONENT_NAME),
                               _component_spawner);

    ct_ebus_a0.disconnect_addr(ECS_EBUS, ECS_COMPONENT_COMPILE,
                               CT_ID64_0(TRANSFORMATION_COMPONENT_NAME),
                               _component_compiler);

    ct_ebus_a0.disconnect(ECS_EBUS, ECS_COMPONENT_CHANGE, on_change);
}

CETECH_MODULE_DEF(
        transform,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_ebus_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)