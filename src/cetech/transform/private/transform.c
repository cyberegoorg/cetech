#include "celib/config.h"
#include "cetech/resource/resource.h"
#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include <celib/yng.h>
#include <celib/ydb.h>
#include <celib/macros.h>
#include <celib/array.inl>
#include <celib/fmath.inl>
#include <celib/ebus.h>
#include <celib/log.h>
#include <celib/cdb.h>

#include "celib/module.h"
#include "cetech/ecs/ecs.h"
#include <cetech/transform/transform.h>
#include <cetech/gfx/debugui.h>
#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>

#include <cetech/editor/editor_ui.h>

#define LOG_WHERE "transform"

static void component_compiler(const char *filename,
                               uint64_t *component_key,
                               uint32_t component_key_count,
                               ce_cdb_obj_o *writer) {
    struct ct_transform_comp t_data;

//    struct ce_yng_doc *d = ce_ydb_a0->get(filename);
    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

//    uint64_t key;

    keys[component_key_count] = ce_yng_a0->key("scale");
//    key = ce_yng_a0->combine_key(keys, CE_ARRAY_LEN(keys));
//    if (d->has_key(d, key)) {
    ce_ydb_a0->get_vec3(filename, keys, CE_ARRAY_LEN(keys),
                        t_data.scale, (float[3]) {});
    ce_cdb_a0->set_vec3(writer, PROP_SCALE, t_data.scale);
//    }


    keys[component_key_count] = ce_yng_a0->key("position");
//    key = ce_yng_a0->combine_key(keys, CE_ARRAY_LEN(keys));
//    if (d->has_key(d, key)) {
    ce_ydb_a0->get_vec3(filename, keys, CE_ARRAY_LEN(keys),
                        t_data.position, (float[3]) {});
    ce_cdb_a0->set_vec3(writer, PROP_POSITION, t_data.position);
//    }

    keys[component_key_count] = ce_yng_a0->key("rotation");
//    key = ce_yng_a0->combine_key(keys, CE_ARRAY_LEN(keys));
//    if (d->has_key(d, key)) {
    ce_ydb_a0->get_vec3(filename, keys, CE_ARRAY_LEN(keys),
                        t_data.rotation, (float[3]) {});
    ce_cdb_a0->set_vec3(writer, PROP_ROTATION, t_data.rotation);
//    }
}

static void transform_transform(struct ct_transform_comp *transform,
                                float *parent) {

    float *pos = transform->position;
    float *rot = transform->rotation;
    float *sca = transform->scale;

    float rot_rad[3];
    ce_vec3_mul_s(rot_rad, rot, CE_DEG_TO_RAD);

    ce_mat4_srt(transform->world,
                sca[0], sca[1], sca[2],
                rot_rad[0], rot_rad[1], rot_rad[2],
                pos[0], pos[1], pos[2]);
}

static struct ct_comp_prop_decs ct_comp_prop_decs = {
        .prop_decs = (struct ct_prop_decs[]) {
                {
                        .type = ECS_PROP_VEC3,
                        .name = PROP_POSITION,
                        .offset = offsetof(struct ct_transform_comp, position),
                },
                {
                        .type = ECS_PROP_VEC3,
                        .name = PROP_ROTATION,
                        .offset = offsetof(struct ct_transform_comp, rotation),
                },
                {
                        .type = ECS_PROP_VEC3,
                        .name = PROP_SCALE,
                        .offset = offsetof(struct ct_transform_comp, scale),
                },
        },
        .prop_n = 3,
};

static const struct ct_comp_prop_decs *prop_desc() {
    return &ct_comp_prop_decs;
}

static const char *prop_display_name(uint64_t prop) {
    switch (prop) {
        case PROP_POSITION:
            return "Position";
        case PROP_ROTATION:
            return "Rotation";
        case PROP_SCALE:
            return "Scale";
        default:
            return NULL;
    }
}

static uint64_t cdb_type() {
    return TRANSFORM_COMPONENT;
}

static const char *display_name() {
    return ICON_FA_ARROWS " Transform";
}

static void guizmo_get_transform(uint64_t obj,
                                 float *world,
                                 float *local) {

    float pos[3] = {};
    float rot[3] = {};
    float sca[3] = {};

    ce_cdb_a0->read_vec3(obj, PROP_POSITION, pos);
    ce_cdb_a0->read_vec3(obj, PROP_ROTATION, rot);
    ce_cdb_a0->read_vec3(obj, PROP_SCALE, sca);

    float rot_rad[3];
    ce_vec3_mul_s(rot_rad, rot, CE_DEG_TO_RAD);

    ce_mat4_srt(world,
                sca[0], sca[1], sca[2],
                rot_rad[0], rot_rad[1], rot_rad[2],
                pos[0], pos[1], pos[2]);
}

static void guizmo_set_transform(uint64_t obj,
                                 uint8_t operation,
                                 float *world,
                                 float *local) {
    float pos[3] = {};
    float rot_deg[3] = {};
    float scale[3] = {};
    ct_debugui_a0->guizmo_decompose_matrix(world, pos, rot_deg, scale);

    struct ct_cdb_obj_t *w = ce_cdb_a0->write_begin(obj);

    switch (operation) {
        case TRANSLATE:
            ce_cdb_a0->set_vec3(w, PROP_POSITION, pos);
            break;

        case ROTATE:
            ce_cdb_a0->set_vec3(w, PROP_ROTATION, rot_deg);
            break;

        case SCALE:
            ce_cdb_a0->set_vec3(w, PROP_SCALE, scale);
            break;

        default:
            break;
    }


    ce_cdb_a0->write_commit(w);
}

static uint64_t create_new() {
    uint64_t component = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                  TRANSFORM_COMPONENT);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(component);
    ce_cdb_a0->set_vec3(w, PROP_SCALE, (float[3]) {1.0f, 1.0f, 1.0f});
    ce_cdb_a0->write_commit(w);

    return component;
}

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
                .prop_display_name = prop_display_name,
                .guizmo_get_transform = guizmo_get_transform,
                .guizmo_set_transform = guizmo_set_transform,
                .create_new = create_new,
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
        .compiler = component_compiler,
        .prop_desc = prop_desc
};


static void foreach_transform(struct ct_world world,
                              struct ct_entity *ent,
                              ct_entity_storage_t *item,
                              uint32_t n,
                              void *data) {
    struct ct_transform_comp *transform;
    transform = ct_ecs_a0->component->get_all(TRANSFORM_COMPONENT, item);

    for (uint32_t i = 1; i < n; ++i) {
        transform_transform(&transform[i], NULL);
    }
}

static void transform_system(struct ct_world world,
                             float dt) {
    uint64_t mask = ct_ecs_a0->component->mask(TRANSFORM_COMPONENT);

    ct_ecs_a0->system->process(world, mask, foreach_transform, &dt);
}

static void _init(struct ce_api_a0 *api) {
    api->register_api(COMPONENT_INTERFACE_NAME, &ct_component_i0);

    ct_ecs_a0->system->register_simulation("transform", transform_system);
}

static void _shutdown() {

}

CE_MODULE_DEF(
        transform,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_yng_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ce_log_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)