//==============================================================================
// Include
//==============================================================================

#include "corelib/allocator.h"

#include "corelib/hashlib.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"
#include "corelib/log.h"
#include <corelib/module.h>
#include <cetech/ecs/entity_property.h>
#include <corelib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <cetech/asset_preview/asset_preview.h>
#include <corelib/macros.h>

//==============================================================================
// GLobals
//==============================================================================

#define _G EntityPreviewGlobals
static struct _G {
    struct ct_entity ent;
} _G;





static int _init(struct ct_api_a0 *api) {
    CT_UNUSED(api);

    _G = (struct _G) {};


    return 1;
}

static void _shutdown() {
    _G = (struct _G) {};
}


CETECH_MODULE_DEF(
        entity_preview,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_asset_preview_a0);
            CETECH_GET_API(api, ct_ecs_a0);

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