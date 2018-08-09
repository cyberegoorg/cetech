//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include <celib/api_system.h>
#include <celib/memory.h>
#include <cetech/ecs/ecs.h>
#include <cetech/resource/resource.h>
#include <celib/os.h>
#include <celib/log.h>
#include <celib/hashlib.h>
#include <celib/module.h>
#include <celib/ydb.h>
#include <celib/hash.inl>
#include <celib/yng.h>

#include "celib/handler.inl"


//==============================================================================
// Globals
//==============================================================================

struct compkey {
    uint64_t keys[64];
};

struct ct_entity_compile_output {
    char *entity_data;
};


//==============================================================================
// Compiler private
//==============================================================================

static void foreach_component(const char *filename,
                              uint64_t *root_key,
                              uint32_t root_count,
                              uint64_t component_key,
                              ce_cdb_obj_o *writer) {

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
                             ce_cdb_obj_o *parent_children_writer) {


    const uint64_t uid = root_key[root_count - 1] ? root_key[root_count - 1]
                                                  : UINT64_MAX;

    uint64_t tmp_keys[root_count + 2];
    memcpy(tmp_keys, root_key, sizeof(uint64_t) * root_count);

    tmp_keys[root_count] = ce_yng_a0->key("name");
    const char *name_str = ce_ydb_a0->get_str(filename, tmp_keys,
                                              root_count + 1, NULL);

    tmp_keys[root_count] = ce_yng_a0->key("PREFAB");
    const char *prefab = ce_ydb_a0->get_str(filename, tmp_keys,
                                            root_count + 1, NULL);

    struct ct_resource_id rid = {{{0}}};
    if (prefab) {
//        compilator_api->add_dependency(filename, prefab);
        ct_resource_a0->type_name_from_filename(prefab, &rid, NULL);

    }

    tmp_keys[root_count] = ce_yng_a0->key("components");
    struct compkey ck = {{0}};
    uint32_t components_keys_count = 0;

    ce_ydb_a0->get_map_keys(filename, tmp_keys, root_count + 1,
                            ck.keys, CE_ARRAY_LEN(ck.keys),
                            &components_keys_count);

    uint64_t obj = ce_cdb_a0->create_object(_G.db, ENTITY_RESOURCE);


    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(obj);

    if (prefab) {
        ce_cdb_a0->set_str(writer, PREFAB_NAME_PROP, prefab);
    }

    uint64_t components_obj = ce_cdb_a0->create_object(_G.db, 0);
    ce_cdb_a0->set_subobject(writer, ENTITY_COMPONENTS, components_obj);

    uint64_t children_obj = ce_cdb_a0->create_object(_G.db, 0);
    ce_cdb_a0->set_subobject(writer, ENTITY_CHILDREN, children_obj);


    if (name_str) {
        ce_cdb_a0->set_str(writer, ENTITY_NAME, name_str);
    }

    ce_cdb_a0->set_uint64(writer, ENTITY_UID, uid);


    ce_cdb_obj_o *components_writer = ce_cdb_a0->write_begin(components_obj);
    for (uint32_t i = 0; i < components_keys_count; ++i) {
        uint64_t comp_obj = ce_cdb_a0->create_object(_G.db,
                                                     ck.keys[i]);
        ce_cdb_obj_o *comp_writer = ce_cdb_a0->write_begin(comp_obj);
        foreach_component(filename,
                          tmp_keys, root_count + 1,
                          ck.keys[i], comp_writer);

        ce_cdb_a0->write_commit(comp_writer);
        ce_cdb_a0->set_subobject(components_writer, ck.keys[i], comp_obj);
    }

    ce_cdb_a0->write_commit(components_writer);

    tmp_keys[root_count] = ce_yng_a0->key("children");

    uint64_t children_keys[32] = {0};
    uint32_t children_keys_count = 0;

    ce_ydb_a0->get_map_keys(filename,
                            tmp_keys, root_count + 1,
                            children_keys, CE_ARRAY_LEN(children_keys),
                            &children_keys_count);

    ce_cdb_obj_o *children_writer = ce_cdb_a0->write_begin(children_obj);
    for (uint32_t i = 0; i < children_keys_count; ++i) {
        tmp_keys[root_count + 1] = children_keys[i];
        compile_entitity(filename, tmp_keys, root_count + 2, 0,
                         parent_obj, output, children_writer);
    }
    ce_cdb_a0->write_commit(children_writer);

    ce_cdb_a0->write_commit(writer);

    if (parent_children_writer) {
        ce_cdb_a0->set_subobject(parent_children_writer, uid, obj);
    } else {
//        uint32_t offset = ce_array_size(output->get_one);
        ce_cdb_a0->dump(obj, &output->entity_data, _G.allocator);
    }
}

static struct ct_entity_compile_output *create_output() {
    struct ct_entity_compile_output *output = CE_ALLOC(_G.allocator,
                                                       struct ct_entity_compile_output,
                                                       sizeof(struct ct_entity_compile_output));
    *output = (struct ct_entity_compile_output) {};

    return output;
}

static void destroy_output(struct ct_entity_compile_output *output) {
    CE_FREE(_G.allocator, output);
}

static void compile_entity(struct ct_entity_compile_output *output,
                           uint64_t *root,
                           uint32_t root_count,
                           const char *filename) {
    compile_entitity(filename, root, root_count, UINT32_MAX, 0, output, NULL);
}

static void write_to_build(struct ct_entity_compile_output *output,
                           const char *filename,
                           char **build) {
    CE_UNUSED(filename);

    ce_array_push_n(*build, output->entity_data,
                    ce_array_size(output->entity_data), _G.allocator);
}

static void _entity_resource_compiler(uint64_t root,
                                      const char *filename,
                                      char **build) {
    struct ct_entity_compile_output *output = create_output();
    compile_entity(output, &root, 1, filename);
    write_to_build(output, filename, build);
    destroy_output(output);
}

static void resource_compiler(const char *filename,
                              char **output) {
    _entity_resource_compiler(0, filename, output);
}
