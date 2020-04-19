#include <stdlib.h>

#include <cetech/property_editor/property_editor.h>
#include <celib/cdb_yaml.h>
#include <celib/math/math.h>

// component
#define PAD_COMPONENT \
    CE_ID64_0("pad", 0x4d967a4a49a1ee61ULL)

static struct ct_ecs_component_i0 pad_component_i = {
        .display_name = "Pad",
        .cdb_type = PAD_COMPONENT,
};
