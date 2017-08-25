//==============================================================================
// Include
//==============================================================================

#include "celib/allocator.h"
#include "celib/map.inl"
#include "celib/buffer.inl"

#include "cetech/hashlib/hashlib.h"
#include "cetech/memory/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/log/log.h"
#include "cetech/os/process.h"
#include "cetech/os/path.h"
#include "cetech/os/vio.h"
#include "cetech/resource/resource.h"
#include <cetech/module/module.h>
#include <cetech/playground/asset_property.h>
#include <cetech/debugui/debugui.h>
#include <cetech/renderer/texture.h>

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


static void texture_asset(uint64_t type,
                          uint64_t name) {
    float size[2];
    ct_debugui_a0.GetWindowSize(size);
    size[1] = size[0];

    ct_debugui_a0.Image2(ct_texture_a0.get(name),
                         size,
                         (float[2]) {0.0f, 0.0f},
                         (float[2]) {1.0f, 1.0f},
                         (float[4]) {1.0f, 1.0f, 1.0f, 1.0f},
                         (float[4]) {0.0f, 0.0f, 0.0, 0.0f});
}

static int _init(ct_api_a0 *api) {
    _G = {};

#if CETECH_CAN_COMPILE
    ct_asset_property_a0.register_asset(
            ct_hash_a0.id64_from_str("texture"),
            texture_asset);
#endif

    return 1;
}

static void _shutdown() {
    _G = {};
}


CETECH_MODULE_DEF(
        texture_property,
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
        },
        {
            _init(api);
        },
        {
            CEL_UNUSED(api);

            _shutdown();
        }
)