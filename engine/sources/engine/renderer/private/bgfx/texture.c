//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <celib/string/stringid.h>
#include "celib/containers/map.h"
#include "celib/os/process.h"
#include "celib/yaml/yaml.h"
#include "celib/filesystem/path.h"
#include "celib/filesystem/vio.h"
#include "celib/filesystem/filesystem.h"
#include <celib/memory/memsys.h>
#include "engine/application/application.h"
#include "engine/resource/resource.h"

//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_texture_handle_t)

MAP_PROTOTYPE(bgfx_texture_handle_t)

struct texture {
    u64 size;
};


//==============================================================================
// GLobals
//==============================================================================

#define _G TextureResourceGlobals
struct G {
    MAP_T(bgfx_texture_handle_t) handler_map;
    stringid64_t type;
} _G = {0};


//==============================================================================
// Compiler private
//==============================================================================

static int _texturec(const char *input,
                     const char *output,
                     int gen_mipmaps,
                     int is_normalmap) {
    char cmd_line[4096] = {0};

    int s = resource_compiler_external_join(cmd_line, CEL_ARRAY_LEN(cmd_line), "texturec");

    s += snprintf(cmd_line + s, CEL_ARRAY_LEN(cmd_line) - s, " -f %s -o %s", input, output);
    if (gen_mipmaps) {
        s += snprintf(cmd_line + s, CEL_ARRAY_LEN(cmd_line) - s, " --mips");
    }

    if (is_normalmap) {
        s += snprintf(cmd_line + s, CEL_ARRAY_LEN(cmd_line) - s, " --normalmap");
    }

    int status = cel_exec(cmd_line);

    log_info("application", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {
    char dir[4096] = {0};
    cel_path_dir(dir, CEL_ARRAY_LEN(dir), filename);

    cel_path_join(tmp_filename, max_len, tmp_dir, dir);
    cel_dir_make_path(tmp_filename);

    return snprintf(tmp_filename, max_len, "%s/%s.ktx", tmp_dir, filename);
}

int _texture_resource_compiler(const char *filename,
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

    char source_data[cel_vio_size(source_vio) + 1];
    memory_set(source_data, 0, cel_vio_size(source_vio) + 1);
    cel_vio_read(source_vio, source_data, sizeof(char), cel_vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t input = yaml_get_node(root, "input");
    yaml_node_t n_gen_mipmaps = yaml_get_node(root, "gen_mipmaps");
    yaml_node_t n_is_normalmap = yaml_get_node(root, "is_normalmap");

    int gen_mipmaps = yaml_is_valid(n_gen_mipmaps) ? yaml_as_bool(n_gen_mipmaps) : 0;
    int is_normalmap = yaml_is_valid(n_is_normalmap) ? yaml_as_bool(n_is_normalmap) : 0;

    const char *source_dir = resource_compiler_get_source_dir();

    resource_compiler_get_build_dir(build_dir, CEL_ARRAY_LEN(build_dir), application_platform());
    resource_compiler_get_tmp_dir(tmp_dir, CEL_ARRAY_LEN(tmp_dir), application_platform());

    yaml_as_string(input, input_str, CEL_ARRAY_LEN(input_str));

    cel_path_join(input_path, CEL_ARRAY_LEN(input_path), source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CEL_ARRAY_LEN(tmp_filename), input_str);

    int result = _texturec(input_path, output_path, gen_mipmaps, is_normalmap);

    if (result != 0) {
        return 0;
    }

    struct vio *tmp_file = cel_vio_from_file(output_path, VIO_OPEN_READ, memsys_main_allocator());
    char *tmp_data = CEL_ALLOCATE(memsys_main_allocator(), char, cel_vio_size(tmp_file) + 1);
    cel_vio_read(tmp_file, tmp_data, sizeof(char), cel_vio_size(tmp_file));

    struct texture resource = {
            .size = cel_vio_size(tmp_file)
    };

    cel_vio_write(build_vio, &resource, sizeof(resource), 1);
    cel_vio_write(build_vio, tmp_data, sizeof(char), resource.size);

    cel_vio_close(tmp_file);
    CEL_DEALLOCATE(memsys_main_allocator(), tmp_data);

    compilator_api->add_dependency(filename, input_str);

    return 1;
}

//==============================================================================
// Resource
//==============================================================================

void *texture_resource_loader(struct vio *input,
                              struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void texture_resource_unloader(void *new_data,
                               struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void texture_resource_online(stringid64_t name,
                             void *data) {
    struct texture *resource = data;

    const bgfx_memory_t *mem = bgfx_copy((resource + 1), resource->size);
    bgfx_texture_handle_t texture = bgfx_create_texture(mem, BGFX_TEXTURE_NONE, 0, NULL);

    MAP_SET(bgfx_texture_handle_t, &_G.handler_map, name.id, texture);
}

static const bgfx_texture_handle_t null_texture = {0};

void texture_resource_offline(stringid64_t name,
                              void *data) {
    bgfx_texture_handle_t texture = MAP_GET(bgfx_texture_handle_t, &_G.handler_map, name.id, null_texture);

    if (texture.idx == null_texture.idx) {
        return;
    }

    bgfx_destroy_texture(texture);

}

void *texture_resource_reloader(stringid64_t name,
                                void *old_data,
                                void *new_data,
                                struct cel_allocator *allocator) {
    texture_resource_offline(name, old_data);
    texture_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t texture_resource_callback = {
        .loader = texture_resource_loader,
        .unloader =texture_resource_unloader,
        .online =texture_resource_online,
        .offline =texture_resource_offline,
        .reloader = texture_resource_reloader
};


//==============================================================================
// Interface
//==============================================================================

int texture_resource_init() {
    _G = (struct G) {0};

    _G.type = stringid64_from_string("texture");

    MAP_INIT(bgfx_texture_handle_t, &_G.handler_map, memsys_main_allocator());

    resource_compiler_register(_G.type, _texture_resource_compiler);

    resource_register_type(_G.type, texture_resource_callback);

    return 1;
}

void texture_resource_shutdown() {
    MAP_DESTROY(bgfx_texture_handle_t, &_G.handler_map);

    _G = (struct G) {0};
}

bgfx_texture_handle_t texture_resource_get(stringid64_t name) {
    resource_get(_G.type, name); // TODO: only for autoload

    return MAP_GET(bgfx_texture_handle_t, &_G.handler_map, name.id, null_texture);
}