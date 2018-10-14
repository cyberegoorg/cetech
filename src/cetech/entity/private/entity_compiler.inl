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
#include <cetech/resource/builddb.h>

#include "celib/handler.inl"


//==============================================================================
// Globals
//==============================================================================


struct ct_entity_compile_output {
    char *entity_data;
};


//==============================================================================
// Compiler private
//==============================================================================

static void foreach_component(const char *filename,
                              uint64_t root_key,
                              uint64_t component_key,
                              ce_cdb_obj_o *writer) {

    uint64_t cid = component_key;
    compile(cid, filename, root_key, writer);
}

static void compile_entitity(const char *filename,
                             uint64_t root_key,
                             uint64_t uid,
                             struct ct_entity_compile_output *output,
                             ce_cdb_obj_o *parent_children_writer) {
//    if(parent_idx =! UINT32_MAX) {
//
//    }

    const char *name_str = ce_cdb_a0->read_str(root_key, ce_ydb_a0->key("name"),
                                               NULL);
    const char *prefab = ce_cdb_a0->read_str(root_key, ce_ydb_a0->key("PREFAB"),
                                             NULL);

    struct ct_resource_id rid = {};
    if (prefab) {
//        compilator_api->add_dependency(filename, prefab);
        ct_resource_a0->type_name_from_filename(prefab, &rid, NULL);

    }

    uint64_t components = ce_cdb_a0->read_subobject(root_key,
                                                    ce_ydb_a0->key(
                                                            "components"), 0);

    const uint64_t components_keys_count = ce_cdb_a0->prop_count(components);
    uint64_t ck[components_keys_count];
    ce_cdb_a0->prop_keys(components, ck);

    uint64_t obj = ce_cdb_a0->create_object(_G.db, ENTITY_RESOURCE);

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(obj);

    if (prefab) {
        ce_cdb_a0->set_str(writer, PREFAB_NAME_PROP, prefab);
    }

    uint64_t components_obj = ce_cdb_a0->create_object(_G.db,
                                                       ENTITY_COMPONENTS);
    ce_cdb_a0->set_subobject(writer, ENTITY_COMPONENTS, components_obj);

    uint64_t children_obj = ce_cdb_a0->create_object(_G.db, ENTITY_CHILDREN);
    ce_cdb_a0->set_subobject(writer, ENTITY_CHILDREN, children_obj);

    if (name_str) {
        ce_cdb_a0->set_str(writer, ENTITY_NAME, name_str);
    }

    ce_cdb_a0->set_uint64(writer, ENTITY_UID, uid);


    ce_cdb_obj_o *components_writer = ce_cdb_a0->write_begin(components_obj);
    for (uint32_t i = 0; i < components_keys_count; ++i) {
        uint64_t comp_obj = ce_cdb_a0->create_object(_G.db, ck[i]);
        ce_cdb_obj_o *comp_writer = ce_cdb_a0->write_begin(comp_obj);

        uint64_t _comp_obj = ce_cdb_a0->read_subobject(components, ck[i], 0);
        foreach_component(filename, _comp_obj, ck[i], comp_writer);

        ce_cdb_a0->write_commit(comp_writer);
        ce_cdb_a0->set_subobject(components_writer, ck[i], comp_obj);
    }

    ce_cdb_a0->write_commit(components_writer);

    uint64_t children = ce_cdb_a0->read_subobject(root_key,
                                                  ce_ydb_a0->key(
                                                          "children"), 0);

    const uint64_t children_keys_count = ce_cdb_a0->prop_count(children);

    uint64_t children_k[children_keys_count];

    ce_cdb_a0->prop_keys(children, children_k);


    ce_cdb_obj_o *children_writer = ce_cdb_a0->write_begin(children_obj);
    for (uint32_t i = 0; i < children_keys_count; ++i) {
        uint64_t ch = ce_cdb_a0->read_subobject(children, children_k[i], 0);
        compile_entitity(filename, ch, children_k[i], output, children_writer);
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


static bool resource_compiler(const char *filename,
                              uint64_t key,
                              struct ct_resource_id rid,
                              const char *fullname) {

    struct ct_entity_compile_output *output = create_output();
    compile_entitity(filename, key,  0, output, NULL);

    ct_builddb_a0->put_resource(fullname, rid, filename, output->entity_data,
                                ce_array_size(output->entity_data));
    destroy_output(output);
    return true;
}
