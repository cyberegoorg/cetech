//==============================================================================
// Include
//==============================================================================

#include "celib/allocator.h"
#include "celib/map.inl"
#include "celib/buffer.inl"

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/log.h"
#include "cetech/kernel/path.h"
#include "cetech/kernel/vio.h"
#include "cetech/kernel/resource.h"
#include <cetech/kernel/module.h>
#include <cetech/modules/playground//asset_property.h>
#include <cetech/modules/debugui/debugui.h>
#include <cetech/modules/renderer/texture.h>
#include <cetech/modules/playground//entity_property.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/modules/transform/transform.h>
#include <cfloat>
#include <celib/fpumath.h>
#include <cetech/kernel/ydb.h>
#include <cetech/modules/playground/command_system.h>

using namespace celib;
using namespace buffer;

//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
static struct _G {
} _G;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_asset_property_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_entity_property_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_cmd_system_a0);


struct set_pos_cmd_s {
    ct_cmd header;

    // ENT
    ct_world world;
    ct_entity entity;

    // ENT YAML
    const char* filename;
    uint64_t keys[32];
    uint32_t keys_count;

    // VALUES
    float new_value[3];
    float old_value[3];
};

static void set_pos_cmd(const struct ct_cmd *cmd,
                        bool inverse) {
    const struct set_pos_cmd_s *pos_cmd = (const set_pos_cmd_s*)cmd;

    const float* value = inverse ? pos_cmd->old_value : pos_cmd->new_value;

    ct_ydb_a0.set_vec3(pos_cmd->filename, pos_cmd->keys, pos_cmd->keys_count, (float*)value);

    ct_transform t = ct_transform_a0.get(pos_cmd->world, pos_cmd->entity);
    ct_transform_a0.set_position(t, (float*)value);
}

static void on_component(struct ct_world world,
                         struct ct_entity entity,
                         const char *filename,
                         uint64_t *keys,
                         uint32_t keys_count) {

    if (!ct_transform_a0.has(world, entity)) {
        return;
    }
    ct_transform t = ct_transform_a0.get(world, entity);


    float pos[3];
    float pos_new[3];
    uint64_t tmp_keys[keys_count + 1];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    tmp_keys[keys_count] = ct_yng_a0.calc_key("position");
    ct_ydb_a0.get_vec3(filename, tmp_keys, keys_count + 1,
                       pos_new, (float[3]) {0.0f});

    vec3_move(pos, pos_new);

    if (ct_debugui_a0.DragFloat3("position", pos_new, 1.0f, -FLT_MAX, FLT_MAX,
                                 "%.3f", 1.0f)) {

        struct set_pos_cmd_s cmd = {
                .header.size = sizeof(struct set_pos_cmd_s),
                .header.type = ct_hash_a0.id64_from_str("transform_set_position"),
                .header.description = {"set transformation position"},
                .world = world,
                .entity = entity,
                .filename = filename,
                .keys_count = keys_count  + 1,
                .new_value = {pos_new[0], pos_new[1], pos_new[2]},
                .old_value = {pos[0], pos[1], pos[2]},
        };
        memcpy(cmd.keys, tmp_keys, sizeof(uint64_t) * cmd.keys_count);
        ct_cmd_system_a0.execute(&cmd.header);
    }

    float rot[4];
    float norm_rot[4];
    float tmp_rot[3];
    ct_transform_a0.get_rotation(t, rot);
    quat_norm(norm_rot, rot);
    quat_to_euler(tmp_rot, norm_rot);
    vec3_mul(tmp_rot, tmp_rot, RAD_TO_DEG);

    tmp_keys[keys_count] = ct_yng_a0.calc_key("rotation");
    ct_ydb_a0.get_vec3(filename, tmp_keys, keys_count + 1, tmp_rot,
                       (float[3]) {0.0f});
    if (ct_debugui_a0.DragFloat3("rotation", tmp_rot, 1.0f, 0, 360, "%.5f",
                                 1.0f)) {
        ct_ydb_a0.set_vec3(filename, tmp_keys, keys_count + 1, tmp_rot);

        float rad_rot[3];
        float q[4];
        vec3_mul(rad_rot, tmp_rot, DEG_TO_RAD);
        quat_from_euler(q, rad_rot[0], rad_rot[1], rad_rot[2]);
        quat_norm(norm_rot, q);
        ct_transform_a0.set_rotation(t, norm_rot);
    }

    float scale[3];
    tmp_keys[keys_count] = ct_yng_a0.calc_key("scale");
    ct_ydb_a0.get_vec3(filename, tmp_keys, keys_count + 1, scale,
                       (float[3]) {1.0f});
    if (ct_debugui_a0.DragFloat3("scale", scale, 1.0f,
                                 -FLT_MAX, FLT_MAX,
                                 "%.3f", 1.0f)) {

        ct_transform_a0.set_scale(t, scale);
        ct_ydb_a0.set_vec3(filename, tmp_keys, keys_count + 1, scale);
    }
}

static int _init(ct_api_a0 *api) {
    CEL_UNUSED(api);

    _G = {};

    ct_entity_property_a0.register_component(
            ct_hash_a0.id64_from_str("transform"), on_component);

    ct_cmd_system_a0.register_cmd_execute(
            ct_hash_a0.id64_from_str("transform_set_position"),
            set_pos_cmd);

    return 1;
}

static void _shutdown() {

    ct_entity_property_a0.unregister_component(
            ct_hash_a0.id64_from_str("transform"));

    _G = {};
}


CETECH_MODULE_DEF(
        transform_property,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_asset_property_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_texture_a0);
            CETECH_GET_API(api, ct_entity_property_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_cmd_system_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            _shutdown();
        }
)