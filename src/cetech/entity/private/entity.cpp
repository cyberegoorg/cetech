//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <cetech/api/api_system.h>
#include <cetech/os/memory.h>
#include <cetech/entity/entity.h>
#include <cetech/resource/resource.h>
#include <cetech/os/path.h>
#include <cetech/log/log.h>
#include <cetech/os/vio.h>
#include <cetech/hashlib/hashlib.h>
#include <cetech/os/errors.h>
#include <cetech/module/module.h>
#include <cetech/yaml/ydb.h>

#include "celib/handler.inl"
#include "celib/map.inl"


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_component_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);

using namespace celib;

//==============================================================================
// Globals
//==============================================================================

#define _G EntityMaagerGlobals

struct entity_instance {
    ct_world world;
    ct_entity* entity;
    uint64_t* guid;
    uint32_t entity_count;
    uint64_t name;
};

static struct _G {
    Handler<uint32_t> entity_handler;

    Map<uint32_t> resource_map;
    Map<uint32_t> spawned_map;
    entity_instance* spawned_array;

    uint64_t type;
    uint64_t level_type;

    cel_alloc* allocator;
} _G;

struct entity_resource {
    uint32_t ent_count;
    uint32_t comp_type_count;
    //uint64_t guid [ent_count]
    //uint64_t parents [ent_count]
    //uint64_t comp_types [comp_type_count]
    //component_data cdata[comp_type_count]
};

struct component_data {
    uint32_t ent_count;
    uint32_t size;
    // uint32_t ent[ent_count];
    // char data[ent_count];
};

#define entity_resource_guid(r) ((uint64_t*)((r) + 1))
#define entity_resource_parents(r) ((uint32_t*)(entity_resource_guid(r) + ((r)->ent_count)))
#define entity_resource_comp_types(r) ((uint64_t*)(entity_resource_parents(r) + ((r)->ent_count)))
#define entity_resource_comp_data(r) ((struct component_data*)(entity_resource_comp_types(r) + ((r)->comp_type_count)))

#define component_data_ent(cd) ((uint32_t*)((cd) + 1))
#define component_data_data(cd) ((char*)((component_data_ent(cd) + ((cd)->ent_count))))


entity_instance* get_spawned_entity(ct_entity ent) {
    uint32_t idx = map::get(_G.spawned_map, ent.h, UINT32_MAX);

    if(UINT32_MAX == idx) {
        return NULL;
    }

    return &_G.spawned_array[idx];
}



ct_entity create() {
    return (ct_entity) {.h = handler::create(_G.entity_handler)};
}

void destroy(ct_world world,
             ct_entity *entity,
             uint32_t count) {

    for (uint32_t i = 0; i < count; ++i) {
        entity_instance* instance = get_spawned_entity(entity[i]);
        if(!instance) {
            continue;
        }

        ct_component_a0.destroy(world, instance->entity, instance->entity_count);

        auto it = multi_map::find_first(_G.resource_map, instance->name);
        while (it != nullptr) {
            struct entity_instance *inst = &_G.spawned_array[it->value];

            if( instance == inst) {
                multi_map::remove(_G.resource_map, it);
                break;
            }

            it = multi_map::find_next(_G.resource_map, it);
        }

        map::remove(_G.spawned_map, entity[i].h);
        handler::destroy(_G.entity_handler, entity[i].h);

        CEL_FREE(ct_memory_a0.main_allocator(), instance->entity);
    }
}

entity_instance * _new_entity(uint64_t name, ct_entity root) {
    uint32_t idx = cel_array_size(_G.spawned_array);
    cel_array_push(_G.spawned_array, {}, _G.allocator);

    entity_instance *instance = &_G.spawned_array[idx];
    instance->name = name;

    multi_map::insert(_G.resource_map, name, idx);
    map::set(_G.spawned_map, root.h, idx);
    return instance;
}

struct entity_instance *get_entity_instance(ct_entity entity) {
    uint32_t idx =map::get(_G.spawned_map, entity.h, UINT32_MAX);

    if(UINT32_MAX == idx) {
        return NULL;
    }

    return &_G.spawned_array[idx];
}

ct_entity spawn_from_resource(ct_world world,
                              entity_instance *instance,
                              entity_resource *resource,
                              ct_entity *spawned) {

    entity_resource *res = resource;

    uint32_t *parents = entity_resource_parents(res);
    uint64_t *comp_types = entity_resource_comp_types(res);

    struct component_data *comp_data = entity_resource_comp_data(res);
    for (uint32_t i = 0; i < res->comp_type_count; ++i) {
        uint64_t type = comp_types[i];

        uint32_t *c_ent = component_data_ent(comp_data);
        char *c_data = component_data_data(comp_data);

        ct_component_a0.spawn(world, type, &spawned[0],
                              c_ent, parents, comp_data->ent_count,
                              c_data);

        comp_data = (struct component_data *) (c_data + comp_data->size);
    }

    entity_instance *se = instance;
    se->world = world;
    se->entity = spawned;
    se->entity_count = res->ent_count;
    se->guid = entity_resource_guid(res);

    return spawned[0];
}


uint32_t find_by_guid(uint64_t* guids, uint32_t guids_count, uint64_t guid) {
    for (int i = 0; i < guids_count; ++i) {
        if(guids[i] != guid ) {
            continue;
        }

        return i;
    }

    return UINT32_MAX;
}

void reload_instance(uint64_t name, void *data) {
    entity_resource *ent_res = (entity_resource *) data;

    auto it = multi_map::find_first(_G.resource_map, name);
    while (it != nullptr) {
        struct entity_instance *instance = &_G.spawned_array[it->value];
        //struct entity_instance old_instance = *instance;

        ct_component_a0.destroy(instance->world, instance->entity, instance->entity_count);

        ct_entity *spawned = CEL_ALLOCATE(ct_memory_a0.main_allocator(),
                                          ct_entity, sizeof(ct_entity) *
                                                     ent_res->ent_count);

        uint64_t* guid = entity_resource_guid(ent_res);
        for (uint32_t j = 0; j < ent_res->ent_count; ++j) {
            uint32_t idx = find_by_guid(instance->guid, instance->entity_count, guid[j]);
            if(UINT32_MAX == idx) {
                spawned[j] = create();
            } else {
                spawned[j] = instance->entity[idx];
            }
        }

        spawn_from_resource(instance->world, instance, ent_res, spawned);

        it = multi_map::find_next(_G.resource_map, it);
    }
}

struct compkey {
    uint64_t keys[32];
    uint32_t count;
};

//==============================================================================
// Compiler private
//==============================================================================
struct ct_entity_compile_output {
    Map<uint32_t> component_ent;
    uint32_t** component_ent_array;
    Map<uint32_t> entity_parent;
    Map<uint32_t> component_body;
    compkey** component_key_array;
    uint64_t* component_type;
    uint64_t* guid;
    uint32_t ent_counter;
};

namespace entity_resource_compiler {
    struct foreach_children_data {
        int parent_ent;
        ct_entity_compile_output *output;
    };


    static void compile_entitity(const char *filename,
                                    uint64_t *root_key,
                                    uint32_t root_count,
                                    unsigned int parent,
                                    ct_entity_compile_output *output,
                                    ct_compilator_api *pApi);

    struct foreach_componets_data {
        ct_entity_compile_output *output;
        uint32_t ent_id;
    };

    void foreach_components_clb(const char *filename,
                                uint64_t *root_key,
                                uint32_t  root_count,
                                uint64_t component_key,
                                struct foreach_componets_data *data) {

        uint64_t cid;
        int contain_cid = 0;

        ct_entity_compile_output *output = data->output;

        uint64_t tmp_keys[root_count + 2];
        memcpy(tmp_keys, root_key, sizeof(uint64_t) * root_count);
        tmp_keys[root_count] = component_key;
        tmp_keys[root_count + 1] = ct_yng_a0.calc_key("component_type");

        const char *component_type = ct_ydb_a0.get_string(
                filename, tmp_keys, root_count + 2, NULL);

        if (!component_type) {
            return;
        }

        cid = CT_ID64_0(component_type);
        for (uint32_t i = 0; i < cel_array_size(output->component_type); ++i) {
            if (output->component_type[i] != cid) {
                continue;
            }

            contain_cid = 1;
        }

        if (!contain_cid) {
            cel_array_push(output->component_type, cid, _G.allocator);

            uint32_t idx = cel_array_size(output->component_ent_array);
            //Array<uint32_t> tmp_a(ct_memory_a0.main_allocator());
            cel_array_push(output->component_ent_array, {}, _G.allocator);
            output->component_ent_array[idx] = NULL;

            map::set(output->component_ent, cid, idx);
        }

        if (!map::has(output->component_body, cid)) {
            uint32_t idx = cel_array_size(output->component_key_array);
            //Array<yaml_node_t> tmp_a(ct_memory_a0.main_allocator());
            cel_array_push(output->component_key_array, {}, _G.allocator);
            output->component_key_array[idx] = NULL;

            map::set(output->component_body, cid, idx);
        }

        uint32_t idx = map::get(output->component_ent, cid, UINT32_MAX);
        cel_array_push(output->component_ent_array[idx], data->ent_id, _G.allocator);

        idx = map::get(output->component_body, cid, UINT32_MAX);

        compkey ck;
        memcpy(ck.keys, tmp_keys, sizeof(uint64_t) * (root_count + 1));
        ck.count =root_count + 1;

        cel_array_push(output->component_key_array[idx], ck, _G.allocator);
    }

    void compile_entitity(const char *filename,
                             uint64_t *root_key,
                             uint32_t root_count,
                             unsigned int parent,
                             ct_entity_compile_output *output,
                             ct_compilator_api *compilator_api) {

        uint32_t ent_id = output->ent_counter++;

        map::set(output->entity_parent, ent_id, (uint32_t) parent);

        uint64_t guid = root_key[root_count-1];
        cel_array_push(output->guid, guid, _G.allocator);

        uint64_t tmp_keys[root_count + 2];
        memcpy(tmp_keys, root_key, sizeof(uint64_t) * root_count);
        tmp_keys[root_count] = ct_yng_a0.calc_key("components");

        struct foreach_componets_data data = {
                .ent_id = ent_id,
                .output = output
        };


        uint64_t components_keys[32] = {};
        uint32_t components_keys_count = 0;

        ct_ydb_a0.get_map_keys(
                filename,
                tmp_keys, root_count + 1,
                components_keys, CETECH_ARRAY_LEN(components_keys),
                &components_keys_count);

        for (uint32_t i = 0; i < components_keys_count; ++i) {
            foreach_components_clb(
                    filename,
                    tmp_keys, root_count + 1,
                    components_keys[i], &data);
        }


        tmp_keys[root_count] = ct_yng_a0.calc_key("children");

        uint64_t children_keys[32] = {};
        uint32_t children_keys_count = 0;

        ct_ydb_a0.get_map_keys(filename,
                               tmp_keys, root_count + 1,
                               children_keys, CETECH_ARRAY_LEN(children_keys),
                               &children_keys_count);

        for (uint32_t i = 0; i < children_keys_count; ++i) {
            int parent_ent = ent_id;
            //output->ent_counter += 1;

            tmp_keys[root_count+1] = children_keys[i];

            compile_entitity(filename, tmp_keys, root_count + 2, parent_ent,
                             output, nullptr);
        }
    }


    ct_entity_compile_output *create_output() {
        cel_alloc *a = ct_memory_a0.main_allocator();

        ct_entity_compile_output *output = CEL_ALLOCATE(a,
                                                        ct_entity_compile_output,
                                                        sizeof(ct_entity_compile_output));
        *output = {0};

        output->component_ent.init(a);
        output->entity_parent.init(a);
        output->component_body.init(a);

        return output;
    }

    void destroy_output(ct_entity_compile_output *output) {
        cel_array_free(output->component_type, _G.allocator);
        cel_array_free(output->guid, _G.allocator);
        output->entity_parent.destroy();

        // clean inner array
        auto ct_it = output->component_ent_array;
        auto ct_end =output->component_ent_array;
        while (ct_it != ct_end) {
            cel_array_free(*ct_it, _G.allocator);
            ++ct_it;
        }

        output->component_ent.destroy();
        cel_array_free(output->component_ent_array, _G.allocator);

        // clean inner array
        auto cb_it = output->component_key_array;
        auto cb_end = output->component_key_array;

        while (cb_it != cb_end) {
            cel_array_free(*cb_it, _G.allocator);
            ++cb_it;
        }

        output->component_body.destroy();
        cel_array_free(output->component_key_array, _G.allocator);

        cel_alloc *a = ct_memory_a0.main_allocator();
        CEL_FREE(a, output);
    }

    void compile_entity(ct_entity_compile_output *output,
                        uint64_t* root,
                        uint32_t root_count,
                        const char *filename,
                        ct_compilator_api *compilator_api) {
        CEL_UNUSED(compilator_api);

        compile_entitity(filename, root, root_count, UINT32_MAX, output,
                         compilator_api);
    }

    uint32_t ent_counter(ct_entity_compile_output *output) {
        return output->ent_counter;
    }

    void write_to_build(ct_entity_compile_output *output,
                        const char *filename,
                        char** build) {
        struct entity_resource res = {};
        res.ent_count = (uint32_t) (output->ent_counter);
        res.comp_type_count = (uint32_t) cel_array_size(output->component_type);


        cel_array_push_n(*build, &res, sizeof(struct entity_resource), _G.allocator);

        //write guids
        cel_array_push_n(*build, &output->guid[0], sizeof(uint64_t) * res.ent_count, _G.allocator);

        //write parents
        for (uint32_t i = 0; i < res.ent_count; ++i) {
            uint32_t id = map::get(output->entity_parent, i, UINT32_MAX);

            cel_array_push_n(*build, &id, sizeof(id), _G.allocator);
        }

        //write comp types
        cel_array_push_n(*build,
                    (uint8_t *) output->component_type,
                    sizeof(uint64_t) * cel_array_size(output->component_type), _G.allocator);

        //write comp data
        for (uint32_t j = 0; j < res.comp_type_count; ++j) {
            uint64_t cid = output->component_type[j];
            uint64_t id = cid;

            uint32_t idx = map::get(output->component_ent, cid, UINT32_MAX);
            uint32_t* ent_arr = output->component_ent_array[idx];

            struct component_data cdata = {
                    .ent_count = (uint32_t) cel_array_size(ent_arr)
            };

            idx = map::get(output->component_body, cid, UINT32_MAX);
            compkey* body = output->component_key_array[idx];

            char* blob = NULL;

            for (uint32_t i = 0; i < cdata.ent_count; ++i) {
                ct_component_a0.compile(id, filename, body[i].keys, body[i].count, &blob);
            }

            cdata.size = cel_array_size(blob);

            cel_array_push_n(*build, (uint8_t *) &cdata, sizeof(cdata), _G.allocator);
            cel_array_push_n(*build, (uint8_t *) ent_arr, sizeof(uint32_t) * cdata.ent_count, _G.allocator);
            cel_array_push_n(*build, blob, sizeof(uint8_t) * cel_array_size(blob), _G.allocator);

            cel_array_free(blob, _G.allocator);
        }
    }

    void _entity_resource_compiler(uint64_t root,
                                   const char *filename,
                                   char** build,
                                   ct_compilator_api *compilator_api) {
        ct_entity_compile_output *output = create_output();
        compile_entity(output, &root, 1, filename, compilator_api);
        write_to_build(output, filename, build);
        destroy_output(output);
    }

    void resource_compiler(const char *filename,
                          char**output,
                          ct_compilator_api *compilator_api) {
        _entity_resource_compiler(0, filename, output, compilator_api);
    }
}

//==============================================================================
// Resource
//==============================================================================

namespace entity_resorce {

    void *loader(ct_vio *input,
                 cel_alloc *allocator) {
        const int64_t size = input->size(input);
        char *data = CEL_ALLOCATE(allocator, char, size);
        input->read(input, data, 1, size);

        return data;
    }

    void unloader(void *new_data,
                  cel_alloc *allocator) {
        CEL_FREE(allocator, new_data);
    }


    void online(uint64_t name,
                void *data) {
        CEL_UNUSED(name, data);
    }

    void offline(uint64_t name,
                 void *data) {
        CEL_UNUSED(name, data);
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   cel_alloc *allocator) {
        offline(name, old_data);
        online(name, new_data);

        reload_instance(name, new_data);

        CEL_FREE(allocator, old_data);
        return new_data;
    }

    static const ct_resource_callbacks_t callback = {
            .loader = loader,
            .unloader = unloader,
            .online = online,
            .offline = offline,
            .reloader = reloader
    };
}

//==============================================================================
// Public interface
//==============================================================================
namespace entity {
    int alive(ct_entity entity) {
        return handler::alive(_G.entity_handler, entity.h);
    }

    ct_entity spawn_type(ct_world world,
                         uint64_t type,
                         uint64_t name) {

        void *res = ct_resource_a0.get(type, name);

        if (res == NULL) {
            ct_log_a0.error("entity", "Could not spawn entity.");
            return (ct_entity) {.h = 0};
        }


        entity_resource *ent_res = (entity_resource *) res;
        ct_entity *spawned = CEL_ALLOCATE(ct_memory_a0.main_allocator(),
                                          ct_entity, sizeof(ct_entity) *
                                                     ent_res->ent_count);

        for (uint32_t j = 0; j < ent_res->ent_count; ++j) {
            spawned[j] = create();
        }

        entity_instance* ent_inst = _new_entity(name, spawned[0]);

        return spawn_from_resource(world, ent_inst, ent_res, spawned);
    }

    ct_entity spawn_entity(ct_world world,
                           uint64_t name) {
        return spawn_type(world, _G.type, name);
    }

    ct_entity spawn_level(ct_world world, uint64_t name) {
        return spawn_type(world, _G.level_type, name);
    }


    ct_entity find_by_guid(ct_entity root, uint64_t guid) {
        entity_instance *se = get_spawned_entity(root);

        if(se) {
            for (int i = 0; i < se->entity_count; ++i) {
                if(se->guid[i] != guid ) {
                    continue;
                }

                return se->entity[i];
            }
        }

        return {.h=0};
    }
}

namespace entity_module {
    static ct_entity_a0 _api = {
            .create = create,
            .destroy = destroy,
            .alive = entity::alive,
            .spawn = entity::spawn_entity,
            .spawn_level = entity::spawn_level,
            .find_by_guid = entity::find_by_guid,

            .compiler = entity_resource_compiler::resource_compiler,
            .compiler_create_output = entity_resource_compiler::create_output,
            .compiler_destroy_output = entity_resource_compiler::destroy_output,
            .compiler_compile_entity = entity_resource_compiler::compile_entity,
            .compiler_ent_counter = entity_resource_compiler::ent_counter,
            .compiler_write_to_build = entity_resource_compiler::write_to_build,
            .resource_compiler = entity_resource_compiler::_entity_resource_compiler
    };

    static void _init_api(ct_api_a0 *api) {
        api->register_api("ct_entity_a0", &_api);

    }

    static void _init(ct_api_a0 *api) {
        _init_api(api);

        _G = {
                .allocator = ct_memory_a0.main_allocator(),
                .type = CT_ID64_0("entity"),
                .level_type = CT_ID64_0("level"),
        };

        _G.spawned_map.init(ct_memory_a0.main_allocator());

        ct_resource_a0.register_type(_G.type, entity_resorce::callback);
        ct_resource_a0.register_type(_G.level_type, entity_resorce::callback);

        ct_resource_a0.compiler_register(_G.type,
                                         entity_resource_compiler::resource_compiler, true);

        ct_resource_a0.compiler_register(_G.level_type,
                                         entity_resource_compiler::resource_compiler, true);

        _G.entity_handler.init(ct_memory_a0.main_allocator());
        _G.spawned_map.init(ct_memory_a0.main_allocator());
        _G.resource_map.init(ct_memory_a0.main_allocator());
    }

    static void _shutdown() {
        _G.resource_map.destroy();
        _G.spawned_map.destroy();
        _G.entity_handler.destroy();

        cel_array_free(_G.spawned_array, _G.allocator);
    }

}

CETECH_MODULE_DEF(
        entity,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_component_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
        },
        {
            CEL_UNUSED(reload);
            entity_module::_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            entity_module::_shutdown();
        }
)