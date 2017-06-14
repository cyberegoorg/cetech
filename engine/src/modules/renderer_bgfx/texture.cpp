//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>

#include <cetech/core/hash.h>
#include <cetech/core/memory.h>
#include <cetech/core/application.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/core/log.h>
#include <cetech/core/path.h>
#include <cetech/core/vio.h>
#include <cetech/core/process.h>

#include <cetech/modules/resource.h>
#include <cstdio>
#include <cetech/core/yaml.h>

using namespace cetech;

//==============================================================================
// Structs
//==============================================================================

struct texture {
    uint64_t size;
};


//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct G {
    Map<bgfx_texture_handle_t> handler_map;
    uint64_t type;
} _G = {0};


IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(app_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(process_api_v0);
IMPORT_API(log_api_v0);
IMPORT_API(hash_api_v0);

//==============================================================================
// Compiler private
//==============================================================================
#ifdef CETECH_CAN_COMPILE

namespace texture_resource_compiler {

    static int _texturec(const char *input,
                         const char *output,
                         int gen_mipmaps,
                         int is_normalmap) {
        char cmd_line[4096] = {0};

        int s = resource_api_v0.compiler_external_join(cmd_line,
                                                       CETECH_ARRAY_LEN(
                                                               cmd_line),
                                                       "texturec");

        s += snprintf(cmd_line + s, CETECH_ARRAY_LEN(cmd_line) - s,
                      " -f %s -o %s",
                      input, output);
        if (gen_mipmaps) {
            s += snprintf(cmd_line + s, CETECH_ARRAY_LEN(cmd_line) - s,
                          " --mips");
        }

        if (is_normalmap) {
            s += snprintf(cmd_line + s, CETECH_ARRAY_LEN(cmd_line) - s,
                          " --normalmap");
        }

        int status = process_api_v0.exec(cmd_line);

        log_api_v0.info("application", "STATUS %d", status);

        return status;
    }

    static int _gen_tmp_name(char *tmp_filename,
                             const char *tmp_dir,
                             size_t max_len,
                             const char *filename) {
        char dir[4096] = {0};
        path_v0.dir(dir, CETECH_ARRAY_LEN(dir), filename);

        path_v0.join(tmp_filename, max_len, tmp_dir, dir);
        path_v0.make_path(tmp_filename);

        return snprintf(tmp_filename, max_len, "%s/%s.ktx", tmp_dir, filename);
    }

    static int _texture_resource_compiler(const char *filename,
                                          struct vio *source_vio,
                                          struct vio *build_vio,
                                          struct compilator_api *compilator_api) {
        // TODO: temp allocator?
        char build_dir[4096] = {0};
        char tmp_dir[4096] = {0};
        char input_str[1024] = {0};
        char input_path[1024] = {0};
        char output_path[4096] = {0};
        char tmp_filename[4096] = {0};

        char source_data[vio_api_v0.size(source_vio) + 1];
        memset(source_data, 0, vio_api_v0.size(source_vio) + 1);
        vio_api_v0.read(source_vio, source_data, sizeof(char),
                        vio_api_v0.size(source_vio));

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(source_data, &h);

        yaml_node_t input = yaml_get_node(root, "input");
        yaml_node_t n_gen_mipmaps = yaml_get_node(root, "gen_mipmaps");
        yaml_node_t n_is_normalmap = yaml_get_node(root, "is_normalmap");

        int gen_mipmaps = yaml_is_valid(n_gen_mipmaps) ? yaml_as_bool(
                n_gen_mipmaps)
                                                       : 0;
        int is_normalmap = yaml_is_valid(n_is_normalmap) ? yaml_as_bool(
                n_is_normalmap) : 0;

        const char *source_dir = resource_api_v0.compiler_get_source_dir();

        resource_api_v0.compiler_get_build_dir(build_dir,
                                               CETECH_ARRAY_LEN(build_dir),
                                               app_api_v0.platform());
        resource_api_v0.compiler_get_tmp_dir(tmp_dir, CETECH_ARRAY_LEN(tmp_dir),
                                             app_api_v0.platform());

        yaml_as_string(input, input_str, CETECH_ARRAY_LEN(input_str));

        path_v0.join(input_path, CETECH_ARRAY_LEN(input_path), source_dir,
                     input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        int result = _texturec(input_path, output_path, gen_mipmaps,
                               is_normalmap);

        if (result != 0) {
            return 0;
        }

        struct vio *tmp_file = vio_api_v0.from_file(output_path, VIO_OPEN_READ,
                                                    memory_api_v0.main_allocator());
        char *tmp_data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                vio_api_v0.size(tmp_file) + 1);
        vio_api_v0.read(tmp_file, tmp_data, sizeof(char),
                        vio_api_v0.size(tmp_file));

        struct texture resource = {
                .size = (uint64_t) vio_api_v0.size(tmp_file)
        };

        vio_api_v0.write(build_vio, &resource, sizeof(resource), 1);
        vio_api_v0.write(build_vio, tmp_data, sizeof(char), resource.size);

        vio_api_v0.close(tmp_file);
        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), tmp_data);

        compilator_api->add_dependency(filename, input_str);

        return 1;
    }

}

#endif

//==============================================================================
// Resource
//==============================================================================

namespace texture_resource {

    static const bgfx_texture_handle_t null_texture = {0};


    void *_texture_resource_loader(struct vio *input,
                                   struct allocator *allocator) {
        const int64_t size = vio_api_v0.size(input);
        char *data = CETECH_ALLOCATE(allocator, char, size);
        vio_api_v0.read(input, data, 1, size);

        return data;
    }

    void _texture_resource_unloader(void *new_data,
                                    struct allocator *allocator) {
        CETECH_DEALLOCATE(allocator, new_data);
    }

    void _texture_resource_online(uint64_t name,
                                  void *data) {
        struct texture *resource = (texture *) data;

        const bgfx_memory_t *mem = bgfx_copy((resource + 1), resource->size);
        bgfx_texture_handle_t texture = bgfx_create_texture(mem,
                                                            BGFX_TEXTURE_NONE,
                                                            0, NULL);

        map::set(_G.handler_map, name, texture);
    }


    void _texture_resource_offline(uint64_t name,
                                   void *data) {
        bgfx_texture_handle_t texture = map::get(_G.handler_map, name,
                                                 null_texture);

        if (texture.idx == null_texture.idx) {
            return;
        }

        bgfx_destroy_texture(texture);

    }

    void *_texture_resource_reloader(uint64_t name,
                                     void *old_data,
                                     void *new_data,
                                     struct allocator *allocator) {
        _texture_resource_offline(name, old_data);
        _texture_resource_online(name, new_data);

        CETECH_DEALLOCATE(allocator, old_data);

        return new_data;
    }

    static const resource_callbacks_t texture_resource_callback = {
            .loader = _texture_resource_loader,
            .unloader =_texture_resource_unloader,
            .online =_texture_resource_online,
            .offline =_texture_resource_offline,
            .reloader = _texture_resource_reloader
    };

}

//==============================================================================
// Interface
//==============================================================================

int texture_init(struct api_v0 *api) {
    _G = (struct G) {0};

    GET_API(api, memory_api_v0);
    GET_API(api, resource_api_v0);
    GET_API(api, app_api_v0);
    GET_API(api, path_v0);
    GET_API(api, vio_api_v0);
    GET_API(api, process_api_v0);
    GET_API(api, log_api_v0);
    GET_API(api, hash_api_v0);

    _G.type = hash_api_v0.id64_from_str("texture");

    _G.handler_map.init(memory_api_v0.main_allocator());

#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.type,
                                      texture_resource_compiler::_texture_resource_compiler);
#endif

    resource_api_v0.register_type(_G.type,
                                  texture_resource::texture_resource_callback);

    return 1;
}

void texture_shutdown() {
    _G = {0};
}

bgfx_texture_handle_t texture_get(uint64_t name) {
    resource_api_v0.get(_G.type, name); // TODO: only for autoload

    return map::get(_G.handler_map, name, texture_resource::null_texture);
}