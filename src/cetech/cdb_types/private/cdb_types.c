#include <celib/memory/allocator.h>
#include "celib/memory/memory.h"
#include "celib/id.h"
#include "celib/api.h"
#include <celib/macros.h>
#include <celib/cdb.h>
#include <celib/module.h>
#include <cetech/cdb_types/cdb_types.h>

static const ce_cdb_prop_def_t0 vec4_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT},
        {.name = "z", .type = CE_CDB_TYPE_FLOAT},
        {.name = "w", .type = CE_CDB_TYPE_FLOAT},
};

static const ce_cdb_prop_def_t0 vec3_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT},
        {.name = "z", .type = CE_CDB_TYPE_FLOAT},
};

static const ce_cdb_prop_def_t0 vec2_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT},
};

static const ce_cdb_prop_def_t0 color4_prop[] = {
        {.name = "r", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
        {.name = "g", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
        {.name = "b", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
        {.name = "a", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
};


void CE_MODULE_LOAD(cdb_types)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);

    ce_cdb_a0->reg_obj_type(VEC2_CDB_TYPE, vec2_prop, CE_ARRAY_LEN(vec2_prop));
    ce_cdb_a0->reg_obj_type(VEC3_CDB_TYPE, vec3_prop, CE_ARRAY_LEN(vec3_prop));
    ce_cdb_a0->reg_obj_type(VEC4_CDB_TYPE, vec4_prop, CE_ARRAY_LEN(vec4_prop));
    ce_cdb_a0->reg_obj_type(COLOR4_CDB_TYPE, color4_prop, CE_ARRAY_LEN(color4_prop));
}

void CE_MODULE_UNLOAD(cdb_types)(struct ce_api_a0 *api,
                                 int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
