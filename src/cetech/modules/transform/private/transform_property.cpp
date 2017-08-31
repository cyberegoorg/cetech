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

static void on_component(struct ct_world world,
                         struct ct_entity entity) {
    if (!ct_transform_a0.has(world, entity)) {
        return;
    }

    ct_transform t = ct_transform_a0.get(world, entity);

    float pos[3];
    float scale[3];
    float rot[4];
    float norm_rot[4];
    float tmp_rot[3];

    ct_transform_a0.get_position(t, pos);
    if (ct_debugui_a0.DragFloat3("position", pos, 1.0f, -FLT_MAX, FLT_MAX,
                                 "%.3f", 1.0f)) {
        ct_transform_a0.set_position(t, pos);
    }

    ct_transform_a0.get_rotation(t, rot);
    quat_norm(norm_rot, rot);
    quat_to_euler(tmp_rot, norm_rot);
    vec3_mul(tmp_rot, tmp_rot, RAD_TO_DEG);

//    if(tmp_rot[0] < 0.0f) tmp_rot[0] = tmp_rot[0] * -1.0f;
//    if(tmp_rot[1] < 0.0f) tmp_rot[1] = tmp_rot[1] * -1.0f;
//    if(tmp_rot[2] < 0.0f) tmp_rot[2] = tmp_rot[2] * -1.0f;

    if (ct_debugui_a0.DragFloat3("rotation", tmp_rot, 1.0f, -FLT_MAX, FLT_MAX, "%.5f", 1.0f)) {
        //ct_log_a0.debug("foo", "rot %f %f %f", tmp_rot[0], tmp_rot[1], tmp_rot[2]);

        vec3_mul(tmp_rot, tmp_rot, DEG_TO_RAD);
        quat_from_euler(rot, tmp_rot[0], tmp_rot[1], tmp_rot[2]);
        quat_norm(norm_rot, rot);
        ct_transform_a0.set_rotation(t, norm_rot);
    }

    ct_transform_a0.get_scale(t, scale);
    if (ct_debugui_a0.DragFloat3("scale", scale, 1.0f, -FLT_MAX, FLT_MAX,
                                 "%.3f", 1.0f)) {
        ct_transform_a0.set_scale(t, scale);
    }

}

static int _init(ct_api_a0 *api) {
    _G = {};

    ct_entity_property_a0.register_component(on_component);

    return 1;
}

static void _shutdown() {

    ct_entity_property_a0.unregister_component(on_component);

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
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);

            _shutdown();
        }
)