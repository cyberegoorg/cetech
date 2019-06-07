#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

// component
#define PAD_COMPONENT \
    CE_ID64_0("pad", 0x4d967a4a49a1ee61ULL)

static const char *pad_display_name() {
    return "Pad";
}

static struct ct_ecs_component_i0 pad_component_i = {
        .display_name = pad_display_name,
        .cdb_type = PAD_COMPONENT,
};
