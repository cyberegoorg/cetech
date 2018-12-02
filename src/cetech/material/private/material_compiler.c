//==============================================================================
// Include
//==============================================================================
#include <celib/ydb.h>
#include <celib/os.h>
#include "celib/memory.h"
#include "celib/api_system.h"
#include "celib/hashlib.h"
#include <celib/array.inl>
#include <celib/ydb.h>
#include <celib/cdb.h>

#include "cetech/ecs/ecs.h"
#include "cetech/machine/machine.h"
#include <bgfx/defines.h>
#include <cetech/renderer/renderer.h>
#include "cetech/resource/resource.h"

#include <cetech/material/material.h>
#include <cetech/resource/builddb.h>
#include <celib/buffer.inl>
#include "material.h"

#define _G material_compiler_globals

struct _G {
    struct ce_alloc *allocator;
} _G;

void _forach_variable_clb(const char *filename,
                          uint64_t root_key,
                          uint64_t key,
                          ce_cdb_obj_o *variables_obj) {

    uint64_t var_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(var_obj);

    const char *name;
    name = ce_cdb_a0->read_str(root_key, ce_id_a0->id64("name"), "");

    ce_cdb_a0->set_subobject(variables_obj, ce_id_a0->id64(name), var_obj);

    ce_cdb_a0->set_str(w, MATERIAL_VAR_NAME_PROP, name);

    char uniform_name[32];
    strcpy(uniform_name, name);

    const char *type = ce_cdb_a0->read_str(root_key, ce_id_a0->id64("type"),
                                           "");

    struct material_variable mat_var = {};


    if (!strcmp(type, "texture")) {
        uint64_t texture_name = 0;

        //TODO : None ptype?
        if (ce_cdb_a0->prop_exist(root_key, ce_id_a0->id64("value"))) {
            const char *v = ce_cdb_a0->read_str(root_key,
                                                ce_id_a0->id64("value"), "");
            texture_name = ce_id_a0->id64(v);
        }

        mat_var.type = MAT_VAR_TEXTURE;
        mat_var.t = texture_name;

        ce_cdb_a0->set_uint64(w, MATERIAL_VAR_VALUE_PROP, mat_var.t);

    } else if ((!strcmp(type, "color")) || (!strcmp(type, "vec4"))) {
        mat_var.type = (!strcmp(type, "color")) ? MAT_VAR_COLOR4
                                                : MAT_VAR_VEC4;

        float v[4] = {
                ce_cdb_a0->read_float(root_key,
                                      MATERIAL_VAR_VALUE_PROP_X, 1.0f),
                ce_cdb_a0->read_float(root_key,
                                      MATERIAL_VAR_VALUE_PROP_Y, 1.0f),
                ce_cdb_a0->read_float(root_key,
                                      MATERIAL_VAR_VALUE_PROP_Z, 1.0f),
                ce_cdb_a0->read_float(root_key,
                                      MATERIAL_VAR_VALUE_PROP_W, 1.0f)
        };

        ce_cdb_a0->set_float(w,
                             MATERIAL_VAR_VALUE_PROP_X, v[0]);
        ce_cdb_a0->set_float(w,
                             MATERIAL_VAR_VALUE_PROP_Y, v[1]);
        ce_cdb_a0->set_float(w,
                             MATERIAL_VAR_VALUE_PROP_Z, v[2]);
        ce_cdb_a0->set_float(w,
                             MATERIAL_VAR_VALUE_PROP_W, v[3]);
    }
    ce_cdb_a0->set_uint64(w, MATERIAL_VAR_TYPE_PROP, mat_var.type);
    ce_cdb_a0->write_commit(w);
}



void name_from_filename(const char *fullname,
                        char *name) {
    const char *resource_type = ce_os_a0->path->extension(fullname);
    size_t size = strlen(fullname) - strlen(resource_type) - 1;
    memcpy(name, fullname, size);
}

uint64_t material_compiler(const char *filename,
                           uint64_t k,
                           struct ct_resource_id rid,
                           const char *fullname) {

    uint64_t obj = k;

//    if (ce_cdb_a0->prop_exist(k, ce_ydb_a0->key("layers"))) {
//
//        uint64_t layers = ce_cdb_a0->read_subobject(k, ce_ydb_a0->key("layers"),
//                                                    0);
//        const uint64_t layers_keys_count = ce_cdb_a0->prop_count(layers);
//        uint64_t layers_keys[layers_keys_count];
//        ce_cdb_a0->prop_keys(layers, layers_keys);
//
//        for (uint32_t i = 0; i < layers_keys_count; ++i) {
//            uint64_t _layer = ce_cdb_a0->read_subobject(layers, layers_keys[i],
//                                                        0);
//
//            foreach_layer(filename, _layer, layers_keys[i], layer_obj);
//        }
//        ce_cdb_a0->write_commit(w);
//    }

    return obj;
}

int materialcompiler_init(struct ce_api_a0 *api) {
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_os_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_ydb_a0);

    _G = (struct _G) {
            .allocator=ce_memory_a0->system
    };


    return 1;
}