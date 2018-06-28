//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include <corelib/api_system.h>
#include <corelib/memory.h>
#include <cetech/ecs/ecs.h>
#include <cetech/resource/resource.h>
#include <corelib/os.h>
#include <corelib/log.h>
#include <corelib/hashlib.h>
#include <corelib/module.h>
#include <corelib/ydb.h>
#include <corelib/hash.inl>

#include "corelib/handler.h"


//==============================================================================
// Globals
//==============================================================================

struct compkey {
    uint64_t keys[64];
};

struct ct_entity_compile_output {
//    struct ct_hash_t component_ent;
//    uint32_t **component_ent_array;
//    struct ct_hash_t entity_parent;
//    struct ct_hash_t component_body;

    char *entity_data;
//    uint32_t *entity_offset;
//    struct compkey *ent_type;
//    uint32_t *ent_type_count;
//
//    uint64_t *uid;
//    uint32_t *prefab;
//    uint32_t ent_counter;
};


//==============================================================================
// Compiler private
//==============================================================================

static void foreach_component(const char *filename,
                              uint64_t *root_key,
                              uint32_t root_count,
                              uint64_t component_key,
                              ct_cdb_obj_o *writer) {

    uint64_t tmp_keys[root_count + 1];
    memcpy(tmp_keys, root_key, sizeof(uint64_t) * root_count);
    tmp_keys[root_count] = component_key;

    uint64_t cid = component_key;
    compile(cid, filename, tmp_keys, root_count + 1, writer);
}

static void compile_entitity(const char *filename,
                             uint64_t *root_key,
                             uint32_t root_count,
                             int parent_idx,
                             uint64_t parent_obj,
                             struct ct_entity_compile_output *output,
                             struct ct_compilator_api *compilator_api,
                             ct_cdb_obj_o *parent_children_writer) {

    CT_UNUSED(compilator_api);

    const uint64_t uid = root_key[root_count - 1] ? root_key[root_count - 1]
                                                  : UINT64_MAX;

    uint64_t tmp_keys[root_count + 2];
    memcpy(tmp_keys, root_key, sizeof(uint64_t) * root_count);

    tmp_keys[root_count] = ct_yng_a0->key("name");
    const char *name_str = ct_ydb_a0->get_str(filename, tmp_keys,
                                              root_count + 1, NULL);

    tmp_keys[root_count] = ct_yng_a0->key("PREFAB");
    const char *prefab = ct_ydb_a0->get_str(filename, tmp_keys,
                                            root_count + 1, NULL);

    struct ct_resource_id rid = {{{0}}};
    if (prefab) {
//        compilator_api->add_dependency(filename, prefab);
        ct_resource_a0->type_name_from_filename(prefab, &rid, NULL);

    }

    tmp_keys[root_count] = ct_yng_a0->key("components");
    struct compkey ck = {{0}};
    uint32_t components_keys_count = 0;

    ct_ydb_a0->get_map_keys(filename, tmp_keys, root_count + 1,
                            ck.keys, CT_ARRAY_LEN(ck.keys),
                            &components_keys_count);

    uint64_t obj = ct_cdb_a0->create_object(_G.db, CT_ID64_0("entity"));


    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(obj);

    if (prefab) {
        ct_cdb_a0->set_str(writer, PREFAB_NAME_PROP, prefab);
    }

    uint64_t components_obj = ct_cdb_a0->create_object(_G.db, 0);
    ct_cdb_a0->set_subobject(writer, CT_ID64_0("components"), components_obj);

    uint64_t children_obj = ct_cdb_a0->create_object(_G.db, 0);
    ct_cdb_a0->set_subobject(writer, CT_ID64_0("children"), children_obj);


    if (name_str) {
        ct_cdb_a0->set_str(writer, CT_ID64_0("name"), name_str);
    }

    ct_cdb_a0->set_uint64(writer, CT_ID64_0("uid"), uid);

    ct_cdb_obj_o *components_writer = ct_cdb_a0->write_begin(components_obj);
    for (uint32_t i = 0; i < components_keys_count; ++i) {
        uint64_t comp_obj = ct_cdb_a0->create_object(_G.db,
                                                     ck.keys[i]);
        ct_cdb_obj_o *comp_writer = ct_cdb_a0->write_begin(comp_obj);
        foreach_component(filename,
                          tmp_keys, root_count + 1,
                          ck.keys[i], comp_writer);

        ct_cdb_a0->write_commit(comp_writer);
        ct_cdb_a0->set_subobject(components_writer, ck.keys[i], comp_obj);
    }

    ct_cdb_a0->write_commit(components_writer);

    tmp_keys[root_count] = ct_yng_a0->key("children");

    uint64_t children_keys[32] = {0};
    uint32_t children_keys_count = 0;

    ct_ydb_a0->get_map_keys(filename,
                            tmp_keys, root_count + 1,
                            children_keys, CT_ARRAY_LEN(children_keys),
                            &children_keys_count);

    ct_cdb_obj_o *children_writer = ct_cdb_a0->write_begin(children_obj);
    for (uint32_t i = 0; i < children_keys_count; ++i) {
        tmp_keys[root_count + 1] = children_keys[i];
        compile_entitity(filename, tmp_keys, root_count + 2, 0,
                         parent_obj, output, compilator_api, children_writer);
    }
    ct_cdb_a0->write_commit(children_writer);

    ct_cdb_a0->write_commit(writer);

    if (parent_children_writer) {
        ct_cdb_a0->set_subobject(parent_children_writer, uid, obj);
    } else {
//        uint32_t offset = ct_array_size(output->entity_data);
        ct_cdb_a0->dump(obj, &output->entity_data, _G.allocator);
    }
}

static struct ct_entity_compile_output *create_output() {
    struct ct_entity_compile_output *output = CT_ALLOC(_G.allocator,
                                                       struct ct_entity_compile_output,
                                                       sizeof(struct ct_entity_compile_output));
    *output = (struct ct_entity_compile_output) {};

    return output;
}

static void destroy_output(struct ct_entity_compile_output *output) {
    CT_FREE(_G.allocator, output);
}

static void compile_entity(struct ct_entity_compile_output *output,
                           uint64_t *root,
                           uint32_t root_count,
                           const char *filename,
                           struct ct_compilator_api *compilator_api) {
    compile_entitity(filename, root, root_count, UINT32_MAX, 0, output,
                     compilator_api, NULL);
}

static void write_to_build(struct ct_entity_compile_output *output,
                           const char *filename,
                           char **build) {
    CT_UNUSED(filename);

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
