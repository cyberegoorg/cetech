#include "corelib/config.h"
#include "cetech/resource/resource.h"
#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"


#include "cetech/ecs/ecs.h"
#include <cetech/transform/transform.h>
#include <corelib/ydb.h>
#include <cetech/macros.h>
#include <corelib/array.inl>
#include <corelib/fmath.inl>
#include <corelib/ebus.h>
#include <corelib/log.h>
#include <cetech/ecs/entity_property.h>
#include <cetech/debugui/debugui.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>

#include "corelib/module.h"

#define LOG_WHERE "transform"

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
//    CT_FREE(ct_memory_a0->system,
//            _G.world_instances[idx].buffer);
//
//    _G.world_instances[idx] = _G.world_instances[last_idx];
//    ct_hash_add(&_G.world_map, last_world.h, idx, _G.allocator);
//    ct_array_pop_back(_G.world_instances);
//}

void _component_compiler(const char *filename,
                         uint64_t *component_key,
                         uint32_t component_key_count,
                         ct_cdb_obj_o *writer) {
    struct ct_transform_comp t_data;

    struct ct_yng_doc *d = ct_ydb_a0->get(filename);
    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    uint64_t key;

    keys[component_key_count] = ct_yng_a0->key("scale");
    key = ct_yng_a0->combine_key(keys, CT_ARRAY_LEN(keys));
    if (d->has_key(d, key)) {
        ct_ydb_a0->get_vec3(filename, keys, CT_ARRAY_LEN(keys),
                            t_data.scale, (float[3]) {0});
        ct_cdb_a0->set_vec3(writer, PROP_SCALE, t_data.scale);
    }


    keys[component_key_count] = ct_yng_a0->key("position");
    key = ct_yng_a0->combine_key(keys, CT_ARRAY_LEN(keys));
    if (d->has_key(d, key)) {
        ct_ydb_a0->get_vec3(filename, keys, CT_ARRAY_LEN(keys),
                            t_data.position, (float[3]) {0});
        ct_cdb_a0->set_vec3(writer, PROP_POSITION, t_data.position);
    }

    keys[component_key_count] = ct_yng_a0->key("rotation");
    key = ct_yng_a0->combine_key(keys, CT_ARRAY_LEN(keys));
    if (d->has_key(d, key)) {
        ct_ydb_a0->get_vec3(filename, keys, CT_ARRAY_LEN(keys),
                            t_data.rotation, (float[3]) {0});
        ct_cdb_a0->set_vec3(writer, PROP_ROTATION, t_data.rotation);
    }
}

void transform_transform(struct ct_transform_comp *transform,
                         float *parent) {

    float *pos = transform->position;
    float *rot = transform->rotation;
    float *sca = transform->scale;

    float rot_rad[3];
    ct_vec3_mul_s(rot_rad, rot, CT_DEG_TO_RAD);

    ct_mat4_srt(transform->world,
                sca[0], sca[1], sca[2],
                rot_rad[0], rot_rad[1], rot_rad[2],
                pos[0], pos[1], pos[2]);
}


static void _on_component_obj_change(uint64_t obj,
                                     const uint64_t *prop,
                                     uint32_t prop_count,
                                     void *data) {

    uint64_t ent_obj = ct_cdb_a0->parent(ct_cdb_a0->parent(obj));

    struct ct_world world = {
            .h = ct_cdb_a0->read_uint64(ent_obj, CT_ID64_0("world"), 0)
    };

    struct ct_entity ent = {
            .h = ct_cdb_a0->read_uint64(ent_obj, CT_ID64_0("entity"), 0)
    };

    struct ct_transform_comp *transform;
    transform = ct_ecs_a0->component->entity_data(world, TRANSFORM_COMPONENT, ent);

    ct_cdb_a0->read_vec3(obj, PROP_POSITION, transform->position);
    ct_cdb_a0->read_vec3(obj, PROP_ROTATION, transform->rotation);
    ct_cdb_a0->read_vec3(obj, PROP_SCALE, transform->scale);

    transform_transform(transform, NULL);
}


static void _component_spawner(uint64_t obj, void* data) {
    struct ct_transform_comp *transform = data;

    ct_cdb_a0->read_vec3(obj, PROP_POSITION, transform->position);
    ct_cdb_a0->read_vec3(obj, PROP_ROTATION, transform->rotation);
    ct_cdb_a0->read_vec3(obj, PROP_SCALE, transform->scale);

    transform_transform(transform, NULL);

    ct_cdb_a0->register_notify(obj, _on_component_obj_change, NULL);
}

static uint64_t cdb_type() {
    return CT_ID64_0("transform");
}

static const char *display_name() {
    return ICON_FA_ARROWS " Transform";
}

static void property_editor(uint64_t obj) {
    ct_entity_property_a0->ui_vec3(obj,
                                   CT_ID64_0("position"), "Position", 0, 0);

    ct_entity_property_a0->ui_vec3(obj,
                                   CT_ID64_0("rotation"), "Rotation",
                                   -360.0f, 360.0f);

    ct_entity_property_a0->ui_vec3(obj, CT_ID64_0("scale"), "Scale", 0, 0);
}

void guizmo_get_transform(uint64_t obj,
                          float *world,
                          float *local) {

    float pos[3] = {0};
    float rot[3] = {0};
    float sca[3] = {0};

    ct_cdb_a0->read_vec3(obj, PROP_POSITION, pos);
    ct_cdb_a0->read_vec3(obj, PROP_ROTATION, rot);
    ct_cdb_a0->read_vec3(obj, PROP_SCALE, sca);

    float rot_rad[3];
    ct_vec3_mul_s(rot_rad, rot, CT_DEG_TO_RAD);

    ct_mat4_srt(world,
                sca[0], sca[1], sca[2],
                rot_rad[0], rot_rad[1], rot_rad[2],
                pos[0], pos[1], pos[2]);
}

void guizmo_set_transform(uint64_t obj,
                          uint8_t operation,
                          float *world,
                          float *local) {
    float pos[3] = {0};
    float rot_deg[3] = {0};
    float scale[3] = {0};
    ct_debugui_a0->guizmo_decompose_matrix(world, pos, rot_deg, scale);

    struct ct_cdb_obj_t *w = ct_cdb_a0->write_begin(obj);

    switch (operation) {
        case TRANSLATE:
            ct_cdb_a0->set_vec3(w, PROP_POSITION, pos);
            break;

        case ROTATE:
            ct_cdb_a0->set_vec3(w, PROP_ROTATION, rot_deg);
            break;

        case SCALE:
            ct_cdb_a0->set_vec3(w, PROP_SCALE, scale);
            break;

        default:
            break;
    }


    ct_cdb_a0->write_commit(w);
}

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
                .property_editor = property_editor,
                .guizmo_get_transform = guizmo_get_transform,
                .guizmo_set_transform = guizmo_set_transform
        };

        return &ct_editor_component_i0;
    }

    return NULL;
}


static uint64_t size() {
    return sizeof(struct ct_transform_comp);
}


static struct ct_component_i0 ct_component_i0 = {
        .size = size,
        .cdb_type = cdb_type,
        .get_interface = get_interface,
                .compiler = _component_compiler,
                        .spawner = _component_spawner,
};

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->system,
            .type = CT_ID64_0("transform"),
    };

    api->register_api("ct_component_i0", &ct_component_i0);
}

static void _shutdown() {

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
            CETECH_GET_API(api, ct_log_a0);
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