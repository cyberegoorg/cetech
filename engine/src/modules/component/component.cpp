//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/map.inl>

#include <cetech/core/memory.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource.h>

#include <cetech/core/module.h>

#include <cetech/modules/world.h>
#include <cetech/modules/entity.h>
#include <cetech/modules/component.h>
#include <cetech/core/yaml.h>
#include <cetech/core/api.h>

IMPORT_API(memory_api_v0);
IMPORT_API(world_api_v0);

using namespace cetech;

//==============================================================================
// Globals
//==============================================================================

namespace {
    static struct ComponentMaagerGlobals {
        Map<component_compiler_t> compiler_map;
        Map<uint32_t> spawn_order_map;
        Map<component_clb> component_clb;
    } _G = {0};
}


//==============================================================================
// Public interface
//==============================================================================

namespace component {

    void register_compiler(uint64_t type,
                           component_compiler_t compiler,
                           uint32_t spawn_order) {
        map::set(_G.compiler_map, type, compiler);
        map::set(_G.spawn_order_map, type, spawn_order);
    }

    int compile(uint64_t type,
                yaml_node_t body,
                blob_v0 *data) {

        component_compiler_t compiler = map::get<component_compiler_t>(
                _G.compiler_map, type, nullptr);

        if (!compiler) {
            return 0;
        }

        return compiler(body, data);
    }

    uint32_t get_spawn_order(uint64_t type) {
        return map::get(_G.spawn_order_map, type, (uint32_t) 0);
    }

    void register_type(uint64_t type,
                       struct component_clb clb) {
        map::set(_G.component_clb, type, clb);

        world_callbacks_t wclb = {
                .on_created = clb.on_world_create,
                .on_destroy = clb.on_world_destroy,
                .on_update = clb.on_world_update,
        };

        world_api_v0.register_callback(wclb);
    }

    void spawn(world_t world,
               uint64_t type,
               entity_t *ent_ids,
               uint32_t *cent,
               uint32_t *ents_parent,
               uint32_t ent_count,
               void *data) {

        struct component_clb clb = map::get(_G.component_clb, type,
                                            component_clb_null);

        if (!clb.spawner) {
            return;
        }

        clb.spawner(world, ent_ids, cent, ents_parent, ent_count, data);
    }

    void destroy(world_t world,
                 entity_t *ent,
                 uint32_t count) {

        auto ce_it = map::begin(_G.component_clb);
        auto ce_end = map::end(_G.component_clb);

        while (ce_it != ce_end) {
            ce_it->value.destroyer(world, ent, count);
            ++ce_it;
        }
    }

    void set_property(uint64_t type,
                      world_t world,
                      entity_t entity,
                      uint64_t key,
                      struct property_value value) {

        struct component_clb clb = map::get(_G.component_clb,
                                            type, component_clb_null);

        if (!clb.set_property) {
            return;
        }

        clb.set_property(world, entity, key, value);
    }

    property_value get_property(uint64_t type,
                                world_t world,
                                entity_t entity,
                                uint64_t key) {

        struct property_value value = {PROPERTY_INVALID};

        struct component_clb clb = map::get(_G.component_clb,
                                            type, component_clb_null);

        if (!clb.get_property) {
            return (struct property_value) {PROPERTY_INVALID};
        }

        return clb.get_property(world, entity, key);
    }
}

namespace component_module {
    static struct component_api_v0 api = {
            .register_compiler = component::register_compiler,
            .compile = component::compile,
            .spawn_order = component::get_spawn_order,
            .register_type = component::register_type,
            .spawn = component::spawn,
            .destroy = component::destroy,
            .set_property = component::set_property,
            .get_property = component::get_property
    };

    void _init_api(struct api_v0 *api_v0) {
        api_v0->register_api("component_api_v0", &api);
    }

    void _init(struct api_v0 *api_v0) {
        GET_API(api_v0, memory_api_v0);
        GET_API(api_v0, world_api_v0);


        _G = {0};

        _G.compiler_map.init(memory_api_v0.main_allocator());
        _G.spawn_order_map.init(memory_api_v0.main_allocator());
        _G.component_clb.init(memory_api_v0.main_allocator());
    }

    void _shutdown() {
        _G = {0};
    }

    extern "C" void *component_get_module_api(int api) {
        switch (api) {
            case PLUGIN_EXPORT_API_ID: {
                static struct module_export_api_v0 module = {0};

                module.init = _init;
                module.init_api = _init_api;
                module.shutdown = _shutdown;


                return &module;
            }

            default:
                return NULL;
        }
    }

}