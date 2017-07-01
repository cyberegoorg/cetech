//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/array.inl>
#include <cetech/celib/map.inl>
#include <cetech/celib/handler.inl>

#include <cetech/core/hash.h>
#include <cetech/core/memory.h>
#include <cetech/core/module.h>
#include <cetech/core/vio.h>
#include <cetech/core/api.h>
#include <cetech/core/path.h>

#include <cetech/modules/resource.h>
#include <cetech/core/errors.h>
#include <cetech/core/yaml.h>

#include "cetech/modules/renderer.h"
#include "texture.h"
#include "shader.h"

#include "material_blob.h"

IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(hash_api_v0);


using namespace cetech;

//==============================================================================
// Structs
//==============================================================================

#define LOG_WHERE "material"

#define _get_resorce(idx) (_G.material_instance_data[_G.material_instance_offset[(idx)]])

//==============================================================================
// GLobals
//==============================================================================
namespace {
    struct MaterialGlobals {
        Map<uint32_t> material_instace_map;
        Array<uint32_t> material_instance_offset;
        Array<uint8_t> material_instance_data;
        Array<uint32_t> material_instance_uniform_data;

        Handler<uint32_t> material_handler;
        uint64_t type;
    } _G;
}

#define material_blob_uniform_bgfx(r)    ((bgfx_uniform_handle_t*) ((material_blob::vec4f_value(r)+((r)->vec4f_count))))

//==============================================================================
// Compiler private
//==============================================================================
#ifdef CETECH_CAN_COMPILE

namespace material_resource_compiler {
    namespace {
        struct material_compile_output {
            Array<char> uniform_names;
            Array<uint8_t> data;

            uint32_t texture_count;
            uint32_t vec4f_count;
            uint32_t mat33f_count;
            uint32_t mat44f_count;
        };

        void _preprocess(const char *filename,
                         yaml_node_t root,
                         struct compilator_api *capi) {
            auto a = memory_api_v0.main_allocator();

            yaml_node_t parent_node = yaml_get_node(root, "parent");

            if (yaml_is_valid(parent_node)) {
                char prefab_file[256] = {0};
                char prefab_str[256] = {0};
                yaml_as_string(parent_node, prefab_str,
                               CETECH_ARRAY_LEN(prefab_str));
                snprintf(prefab_file, CETECH_ARRAY_LEN(prefab_file),
                         "%s.material",
                         prefab_str);

                capi->add_dependency(filename, prefab_file);

                const char *source_dir = resource_api_v0.compiler_get_source_dir();
                char *full_path = path_v0.join(a, 2, source_dir, prefab_file);

                struct vio *prefab_vio = vio_api_v0.from_file(full_path,
                                                              VIO_OPEN_READ,
                                                              memory_api_v0.main_allocator());

                CETECH_DEALLOCATE(a, full_path);

                char prefab_data[vio_api_v0.size(prefab_vio) + 1];
                memset(prefab_data, 0, vio_api_v0.size(prefab_vio) + 1);
                vio_api_v0.read(prefab_vio, prefab_data, sizeof(char),
                                vio_api_v0.size(prefab_vio));
                vio_api_v0.close(prefab_vio);

                yaml_document_t h;
                yaml_node_t prefab_root = yaml_load_str(prefab_data, &h);

                _preprocess(filename, prefab_root, capi);
                yaml_merge(root, prefab_root);
            }
        }

        void _forach_texture_clb(yaml_node_t key,
                                 yaml_node_t value,
                                 void *_data) {
            struct material_compile_output *output = (material_compile_output *) _data;

            output->texture_count += 1;

            char tmp_buffer[512] = {0};
            char uniform_name[32] = {0};

            yaml_as_string(key, uniform_name,
                           CETECH_ARRAY_LEN(uniform_name) - 1);

            yaml_as_string(value, tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer));
            uint64_t texture_name = hash_api_v0.id64_from_str(tmp_buffer);

            array::push(output->uniform_names, uniform_name,
                        CETECH_ARRAY_LEN(uniform_name));
            array::push(output->data, (uint8_t *) &texture_name,
                        sizeof(uint64_t));
        }

        void _forach_vec4fs_clb(yaml_node_t key,
                                yaml_node_t value,
                                void *_data) {
            struct material_compile_output *output = (material_compile_output *) _data;

            output->vec4f_count += 1;

            char uniform_name[32] = {0};
            yaml_as_string(key, uniform_name,
                           CETECH_ARRAY_LEN(uniform_name) - 1);

            vec4f_t v = yaml_as_vec4f_t(value);

            array::push(output->uniform_names, uniform_name,
                        CETECH_ARRAY_LEN(uniform_name));
            array::push(output->data, (uint8_t *) &v, sizeof(vec4f_t));
        }

        void _forach_mat44f_clb(yaml_node_t key,
                                yaml_node_t value,
                                void *_data) {
            struct material_compile_output *output = (material_compile_output *) _data;

            output->mat44f_count += 1;

            char uniform_name[32] = {0};
            yaml_as_string(key, uniform_name,
                           CETECH_ARRAY_LEN(uniform_name) - 1);

            mat44f_t m = yaml_as_mat44f_t(value);

            array::push(output->uniform_names, uniform_name,
                        CETECH_ARRAY_LEN(uniform_name));
            array::push(output->data, (uint8_t *) &m, sizeof(mat44f_t));
        }

        void _forach_mat33f_clb(yaml_node_t key,
                                yaml_node_t value,
                                void *_data) {
            struct material_compile_output *output = (material_compile_output *) _data;

            output->mat33f_count += 1;

            char uniform_name[32] = {0};
            yaml_as_string(key, uniform_name,
                           CETECH_ARRAY_LEN(uniform_name) - 1);

            mat33f_t m = yaml_as_mat33f_t(value);

            array::push(output->uniform_names, uniform_name,
                        CETECH_ARRAY_LEN(uniform_name));
            array::push(output->data, (uint8_t *) &m, sizeof(mat33f_t));
        }
    }

    int compiler(const char *filename,
                 struct vio *source_vio,
                 struct vio *build_vio,
                 struct compilator_api *compilator_api) {
        char *source_data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                vio_api_v0.size(source_vio) + 1);
        memset(source_data, 0, vio_api_v0.size(source_vio) + 1);

        vio_api_v0.read(source_vio, source_data, sizeof(char),
                        vio_api_v0.size(source_vio));

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(source_data, &h);

        _preprocess(filename, root, compilator_api);

        yaml_node_t shader_node = yaml_get_node(root, "shader");
        CETECH_ASSERT("material", yaml_is_valid(shader_node));

        char tmp_buffer[256] = {0};
        yaml_as_string(shader_node, tmp_buffer, CETECH_ARRAY_LEN(tmp_buffer));

        struct material_compile_output output = {0};
        output.uniform_names.init(memory_api_v0.main_allocator());
        output.data.init(memory_api_v0.main_allocator());

        yaml_node_t textures = yaml_get_node(root, "textures");
        if (yaml_is_valid(textures)) {
            yaml_node_foreach_dict(textures, _forach_texture_clb, &output);
        }

        yaml_node_t vec4 = yaml_get_node(root, "vec4f");
        if (yaml_is_valid(vec4)) {
            yaml_node_foreach_dict(vec4, _forach_vec4fs_clb, &output);
        }

        yaml_node_t mat44 = yaml_get_node(root, "mat44f");
        if (yaml_is_valid(mat44)) {
            yaml_node_foreach_dict(mat44, _forach_mat44f_clb, &output);
        }

        yaml_node_t mat33 = yaml_get_node(root, "mat33f");
        if (yaml_is_valid(mat33)) {
            yaml_node_foreach_dict(mat33, _forach_mat33f_clb, &output);
        }

        material_blob::blob_t resource = {
                .shader_name = hash_api_v0.id64_from_str(tmp_buffer),
                .texture_count =output.texture_count,
                .vec4f_count = output.vec4f_count,
                .uniforms_count = (uint32_t) (
                        array::size(output.uniform_names) / 32),
        };

        vio_api_v0.write(build_vio, &resource, sizeof(resource), 1);
        vio_api_v0.write(build_vio, output.uniform_names._data, sizeof(char),
                         array::size(output.uniform_names));
        vio_api_v0.write(build_vio, output.data._data, sizeof(uint8_t),
                         array::size(output.data));

        output.uniform_names.destroy();
        output.data.destroy();

        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), source_data);
        return 1;
    }
}

#endif

//==============================================================================
// Resource
//==============================================================================
namespace material_resource {
    static const bgfx_program_handle_t null_program = {0};

    void *loader(struct vio *input,
                 struct allocator *allocator) {
        const int64_t size = vio_api_v0.size(input);
        char *data = CETECH_ALLOCATE(allocator, char, size);
        vio_api_v0.read(input, data, 1, size);

        return data;
    }

    void unloader(void *new_data,
                  struct allocator *allocator) {
        CETECH_DEALLOCATE(allocator, new_data);
    }

    void online(uint64_t name,
                void *data) {
    }

    void offline(uint64_t name,
                 void *data) {
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   struct allocator *allocator) {
        offline(name, old_data);
        online(name, new_data);

        CETECH_DEALLOCATE(allocator, old_data);
        return new_data;
    }

}


//==============================================================================
// Interface
//==============================================================================
namespace {
    static const resource_callbacks_t resource_callback = {
            .loader = material_resource::loader,
            .unloader = material_resource::unloader,
            .online = material_resource::online,
            .offline = material_resource::offline,
            .reloader = material_resource::reloader
    };
}

namespace material {

    int init(struct api_v0 *api) {
        GET_API(api, memory_api_v0);
        GET_API(api, resource_api_v0);
        GET_API(api, path_v0);
        GET_API(api, vio_api_v0);
        GET_API(api, hash_api_v0);

        _G = {0};

        _G.type = hash_api_v0.id64_from_str("material");

        _G.material_handler.init(memory_api_v0.main_allocator());

        _G.material_instace_map.init(memory_api_v0.main_allocator());
        _G.material_instance_offset.init(memory_api_v0.main_allocator());
        _G.material_instance_data.init(memory_api_v0.main_allocator());

        resource_api_v0.register_type(_G.type, resource_callback);
#ifdef CETECH_CAN_COMPILE
        resource_api_v0.compiler_register(_G.type,
                                          material_resource_compiler::compiler);
#endif
        return 1;
    }

    void shutdown() {
        _G.material_handler.destroy();

        _G.material_instace_map.destroy();
        _G.material_instance_offset.destroy();
        _G.material_instance_data.destroy();
    }

    static const material_t null_material = {0};

    material_t create(uint64_t name) {
        auto resource = material_blob::get(resource_api_v0.get(_G.type, name));

        uint32_t size = material_blob::size(resource);
        uint32_t h = handler::create(_G.material_handler);

        uint32_t idx = (uint32_t) array::size(_G.material_instance_offset);

        map::set(_G.material_instace_map, h, idx);

        uint32_t offset = array::size(_G.material_instance_data);
        array::push(_G.material_instance_data, (uint8_t *) resource, size);
        array::push_back(_G.material_instance_offset, offset);

        // write bgfx uniform handlers
        bgfx_uniform_handle_t bgfx_uniforms[resource->uniforms_count];
        const char *u_names = (const char *) (resource + 1);

        uint32_t off = 0;
        uint32_t tmp_off = 0;
        off += resource->texture_count;
        for (int i = 0; i < resource->texture_count; ++i) {
            bgfx_uniforms[i] = bgfx_create_uniform(&u_names[i * 32],
                                                   BGFX_UNIFORM_TYPE_INT1, 1);
        }

        tmp_off = off;
        off += resource->vec4f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx_create_uniform(&u_names[i * 32],
                                                   BGFX_UNIFORM_TYPE_VEC4, 1);
        }

        tmp_off = off;
        off += resource->mat33f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx_create_uniform(&u_names[i * 32],
                                                   BGFX_UNIFORM_TYPE_MAT3, 1);
        }

        tmp_off = off;
        off += resource->mat44f_count;
        for (int i = tmp_off; i < off; ++i) {
            bgfx_uniforms[i] = bgfx_create_uniform(&u_names[i * 32],
                                                   BGFX_UNIFORM_TYPE_MAT4, 1);
        }

        array::push(_G.material_instance_data, (uint8_t *) bgfx_uniforms,
                    sizeof(bgfx_uniform_handle_t) * resource->uniforms_count);

        return (material_t) {.idx=h};
    }


    uint32_t get_texture_count(material_t material) {
        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return 0;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        return material_blob::texture_count(resource);
    }

    uint32_t _material_find_slot(const material_blob::blob_t *resource,
                                 const char *name) {

        const char *u_names = material_blob::uniform_names(resource);
        for (uint32_t i = 0; i < resource->uniforms_count; ++i) {
            if (strcmp(&u_names[i * 32], name) != 0) {
                continue;
            }

            return i;
        }

        return UINT32_MAX;
    }

    void set_texture(material_t material,
                     const char *slot,
                     uint64_t texture) {

        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        uint64_t *u_texture = material_blob::texture_names(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_texture[slot_idx] = texture;
    }

    void set_vec4f(material_t material,
                   const char *slot,
                   vec4f_t v) {

        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        vec4f_t *u_vec4f = material_blob::vec4f_value(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_vec4f[slot_idx - (resource->texture_count)] = v;
    }

    void set_mat33f(material_t material,
                    const char *slot,
                    mat33f_t v) {

        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        mat33f_t *u_mat33f = material_blob::mat33f_value(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_mat33f[slot_idx -
                 (resource->texture_count + resource->vec4f_count)] = v;
    }

    void set_mat44f(material_t material,
                    const char *slot,
                    mat44f_t v) {
        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        mat44f_t *u_mat44f = material_blob::mat44f_value(resource);

        int slot_idx = _material_find_slot(resource, slot);

        u_mat44f[slot_idx - (resource->texture_count + resource->vec4f_count +
                             resource->mat33f_count)] = v;
    }


    void use(material_t material) {
        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);

        if (idx == UINT32_MAX) {
            return;
        }

        auto resource = material_blob::get(&_get_resorce(idx));

        uint64_t *u_texture = material_blob::texture_names(resource);
        vec4f_t *u_vec4f = material_blob::vec4f_value(resource);
        mat33f_t *u_mat33f = material_blob::mat33f_value(resource);
        mat44f_t *u_mat44f = material_blob::mat44f_value(resource);

        bgfx_uniform_handle_t *u_handler = material_blob_uniform_bgfx(resource);


        // TODO: refactor: one loop
        uint32_t offset = 0;
        for (int i = 0; i < resource->texture_count; ++i) {
            bgfx_texture_handle_t texture = texture_get(u_texture[i]);
            bgfx_set_texture(i, u_handler[offset + i], texture, 0);
        }
        offset += resource->texture_count;


        for (int i = 0; i < resource->vec4f_count; ++i) {
            bgfx_set_uniform(u_handler[offset + i], &u_vec4f[i], 1);
        }
        offset += resource->vec4f_count;


        for (int i = 0; i < resource->mat33f_count; ++i) {
            bgfx_set_uniform(u_handler[offset + i], &u_mat33f[i], 1);
        }
        offset += resource->mat33f_count;

        for (int i = 0; i < resource->mat44f_count; ++i) {
            bgfx_set_uniform(u_handler[offset + i], &u_mat44f[i], 1);
        }
        offset += resource->mat44f_count;


        uint64_t state = (0
                          | BGFX_STATE_RGB_WRITE
                          | BGFX_STATE_ALPHA_WRITE
                          | BGFX_STATE_DEPTH_TEST_LESS
                          | BGFX_STATE_DEPTH_WRITE
                          | BGFX_STATE_CULL_CCW
                          | BGFX_STATE_MSAA
        );

        bgfx_set_state(state, 0);
    }

    void submit(material_t material) {
        uint32_t idx = map::get(_G.material_instace_map, material.idx,
                                UINT32_MAX);
        CETECH_ASSERT(LOG_WHERE, idx != UINT32_MAX);

        auto resource = material_blob::get(&_get_resorce(idx));

        bgfx_submit(0, shader_get(material_blob::shader_name(resource)), 0, 0);
    }

}