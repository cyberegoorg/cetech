//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/engine/world/world.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/core/os/path.h>
#include <cetech/core/log/log.h>
#include <cetech/core/os/vio.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/os/errors.h>
#include <cetech/core/module/module.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/containers/hash.h>

#include "cetech/core/containers/handler.h"


//==============================================================================
// Globals
//==============================================================================

struct compkey {
    uint64_t keys[64];
};

struct ct_entity_compile_output {
    struct ct_hash_t component_ent;
    uint32_t **component_ent_array;
    struct ct_hash_t entity_parent;
    struct ct_hash_t component_body;

    char *entity_data;
    uint32_t *entity_offset;
    struct compkey *ent_type;
    uint32_t *ent_type_count;

    uint64_t *uid;
    uint64_t *prefab;
    uint64_t *prefab_type;
    uint32_t ent_counter;
};


//==============================================================================
// Compiler private
//==============================================================================

static void foreach_component(const char *filename,
                              uint64_t *root_key,
                              uint32_t root_count,
                              uint64_t component_key,
                              struct ct_cdb_writer_t *writer) {

    uint64_t tmp_keys[root_count + 1];
    memcpy(tmp_keys, root_key, sizeof(uint64_t) * root_count);
    tmp_keys[root_count] = component_key;

    uint64_t cid = component_key;
    compile(cid, filename, tmp_keys, root_count + 1, writer);
}

static void compile_entitity(const char *filename,
                             uint64_t *root_key,
                             uint32_t root_count,
                             unsigned int parent,
                             struct ct_cdb_obj_t* parent_obj,
                             struct ct_entity_compile_output *output,
                             struct ct_compilator_api *compilator_api) {

    uint32_t ent_id = output->ent_counter++;

    ct_hash_add(&output->entity_parent, ent_id, (uint32_t) parent,
                _G.allocator);

    uint64_t uid = root_key[root_count - 1];
    ct_array_push(output->uid, uid, _G.allocator);

    uint64_t tmp_keys[root_count + 2];
    memcpy(tmp_keys, root_key, sizeof(uint64_t) * root_count);


    struct ct_cdb_obj_t *obj = ct_cdb_a0.create_object();
    struct ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(obj);

    tmp_keys[root_count] = ct_yng_a0.key("PARENT");
    const char *prefab = ct_ydb_a0.get_string(filename, tmp_keys,
                                              root_count + 1, NULL);

    uint64_t type, name;
    type = name = 0;
    if (prefab) {
//        compilator_api->add_dependency(filename, prefab);
        ct_resource_a0.type_name_from_filename(prefab, &type, &name, NULL);
    }

    ct_array_push(output->prefab, name, _G.allocator);
    ct_array_push(output->prefab_type, type, _G.allocator);

    tmp_keys[root_count] = ct_yng_a0.key("components");
    struct compkey ck = {{0}};
    uint32_t components_keys_count = 0;

    ct_ydb_a0.get_map_keys(filename, tmp_keys, root_count + 1,
                           ck.keys, CETECH_ARRAY_LEN(ck.keys),
                           &components_keys_count);

    ct_array_push(output->ent_type, ck, _G.allocator);
    ct_array_push(output->ent_type_count, components_keys_count, _G.allocator);


    for (uint32_t i = 0; i < components_keys_count; ++i) {
        foreach_component(filename,
                          tmp_keys, root_count + 1,
                          ck.keys[i], writer);
    }
    ct_cdb_a0.write_commit(writer);

    uint32_t offset = ct_array_size(output->entity_data);
    ct_cdb_a0.dump(obj, &output->entity_data, _G.allocator);
    ct_array_push(output->entity_offset, offset, _G.allocator);

    tmp_keys[root_count] = ct_yng_a0.key("children");

    uint64_t children_keys[32] = {0};
    uint32_t children_keys_count = 0;

    ct_ydb_a0.get_map_keys(filename,
                           tmp_keys, root_count + 1,
                           children_keys, CETECH_ARRAY_LEN(children_keys),
                           &children_keys_count);

    for (uint32_t i = 0; i < children_keys_count; ++i) {
        int parent_ent = ent_id;
        tmp_keys[root_count + 1] = children_keys[i];
        compile_entitity(filename, tmp_keys, root_count + 2,
                         parent_ent, obj, output, NULL);
    }


}

static struct ct_entity_compile_output *create_output() {
    struct ct_alloc *a = ct_memory_a0.main_allocator();

    struct ct_entity_compile_output *output = CT_ALLOC(a,
                                                       struct ct_entity_compile_output,
                                                       sizeof(struct ct_entity_compile_output));
    *output = (struct ct_entity_compile_output) {};

    return output;
}

static void destroy_output(struct ct_entity_compile_output *output) {
    ct_array_free(output->uid, _G.allocator);
    ct_hash_free(&output->entity_parent, _G.allocator);

    // clean inner array
    uint32_t **ct_it = output->component_ent_array;
    uint32_t **ct_end = output->component_ent_array +
                        ct_array_size(output->component_ent_array);
    while (ct_it != ct_end) {
        ct_array_free(*ct_it, _G.allocator);
        ++ct_it;
    }

    ct_hash_free(&output->component_ent, _G.allocator);
    ct_array_free(output->component_ent_array, _G.allocator);


    ct_hash_free(&output->component_body, _G.allocator);

    struct ct_alloc *a = ct_memory_a0.main_allocator();
    CT_FREE(a, output);
}

static void compile_entity(struct ct_entity_compile_output *output,
                           uint64_t *root,
                           uint32_t root_count,
                           const char *filename,
                           struct ct_compilator_api *compilator_api) {
    CT_UNUSED(compilator_api);

    compile_entitity(filename, root, root_count, UINT32_MAX, NULL, output,
                     compilator_api);
}

static void write_to_build(struct ct_entity_compile_output *output,
                           const char *filename,
                           char **build) {
    struct entity_resource res = (struct entity_resource) {0};
    res.ent_count = (uint32_t) (output->ent_counter);


    ct_array_push_n(*build, &res, sizeof(struct entity_resource), _G.allocator);

    //write uids
    ct_array_push_n(*build, &output->uid[0], sizeof(uint64_t) * res.ent_count,
                    _G.allocator);

    //write parents
    for (uint32_t i = 0; i < res.ent_count; ++i) {
        uint32_t id = ct_hash_lookup(&output->entity_parent, i, UINT32_MAX);

        ct_array_push_n(*build, &id, sizeof(id), _G.allocator);
    }


    ct_array_push_n(*build, output->ent_type,
                    sizeof(struct compkey) * ct_array_size(output->ent_type),
                    _G.allocator);

    ct_array_push_n(*build, output->ent_type_count,
                    sizeof(uint32_t) * ct_array_size(output->ent_type_count),
                    _G.allocator);

    ct_array_push_n(*build, output->entity_offset,
                    sizeof(uint32_t) * ct_array_size(output->entity_offset),
                    _G.allocator);

    ct_array_push_n(*build, output->prefab,
                    sizeof(uint64_t) * ct_array_size(output->prefab),
                    _G.allocator);

    ct_array_push_n(*build, output->prefab_type,
                    sizeof(uint64_t) * ct_array_size(output->prefab_type),
                    _G.allocator);

    ct_array_push_n(*build, output->entity_data,
                    ct_array_size(output->entity_data), _G.allocator);
}

static void _entity_resource_compiler(uint64_t root,
                                      const char *filename,
                                      char **build,
                                      struct ct_compilator_api *compilator_api) {
    struct ct_entity_compile_output *output = create_output();
    compile_entity(output, &root, 1, filename, compilator_api);
    write_to_build(output, filename, build);
    destroy_output(output);
}

static void resource_compiler(const char *filename,
                              char **output,
                              struct ct_compilator_api *compilator_api) {
    _entity_resource_compiler(0, filename, output, compilator_api);
}
