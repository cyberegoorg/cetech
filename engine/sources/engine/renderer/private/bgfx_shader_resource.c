//==============================================================================
// Include
//==============================================================================

#include <bgfx/c99/bgfx.h>

#include <celib/stringid/stringid.h>
#include "celib/containers/array.h"
#include "celib/containers/map.h"
#include "celib/os/process.h"
#include "celib/yaml/yaml.h"
#include "celib/os/path.h"
#include "celib/os/vio.h"
#include "celib/os/fs.h"

#include "engine/memory_system/memory_system.h"
#include "engine/application/application.h"
#include "engine/resource_manager/resource_manager.h"
#include "engine/resource_compiler/resource_compiler.h"


//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_program_handle_t)

MAP_PROTOTYPE(bgfx_program_handle_t)

struct shader_resource {
    u64 vs_size;
    u64 fs_size;
    // u8 vs [vs_size]
    // u8 fs [fs_size]
};


//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct G {
    MAP_T(bgfx_program_handle_t) handler_map;
    stringid64_t shader_type;
} _G = {0};


//==============================================================================
// Compiler private
//==============================================================================


static int _shaderc(const char *input,
                    const char *output,
                    const char *include_path,
                    const char *type,
                    const char *platform,
                    const char *profile) {
    char cmd_line[4096] = {0};
    int s = snprintf(cmd_line, CE_ARRAY_LEN(cmd_line),
                     "externals/build/linux64/release/bin/shaderc"
                             " -f %s"
                             " -o %s"
                             " -i %s"
                             " --type %s"
                             " --platform %s"
                             " --profile %s"

                             " 2>&1",  // TODO: move to os_exec

                     input, output, include_path, type, platform, profile);

    int status = os_exec(cmd_line);

    log_debug("shaderc", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {
    char dir[1024] = {0};
    os_path_dir(dir, CE_ARRAY_LEN(dir), filename);

    os_path_join(tmp_filename, max_len, tmp_dir, dir);
    os_dir_make_path(tmp_filename);

    return snprintf(tmp_filename, max_len, "%s/%s.shaderc", tmp_dir, filename);
}


#if defined(CETECH_LINUX)
const char *platform = "linux";
const char *vs_profile = "120";
const char *fs_profile = "120";
#elif defined(CETECH_DARWIN)
const char* platform = "osx";
const char* vs_profile = "120";
const char* fs_profile = "120";
#else
const char* platform = "windows";
const char* vs_profile = "vs_4_0";
const char* fs_profile = "ps_4_0";
#endif

int _shader_resource_compiler(const char *filename,
                              struct vio *source_vio,
                              struct vio *build_vio,
                              struct compilator_api *compilator_api) {

    char source_data[vio_size(source_vio) + 1];
    memory_set(source_data, 0, vio_size(source_vio) + 1);
    vio_read(source_vio, source_data, sizeof(char), vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t vs_input = yaml_get_node(root, "vs_input");
    yaml_node_t fs_input = yaml_get_node(root, "fs_input");

    const char *source_dir = resource_compiler_get_source_dir();
    const char *core_dir = resource_compiler_get_core_dir();

    char include_dir[1024] = {0};
    os_path_join(include_dir, CE_ARRAY_LEN(include_dir), core_dir, "bgfxshaders");


    struct shader_resource resource = {0};

    // TODO: temp allocator?
    char build_dir[256] = {0};
    char tmp_dir[256] = {0};
    char input_str[256] = {0};
    char input_path[256] = {0};
    char output_path[256] = {0};
    char tmp_filename[256] = {0};

    resource_compiler_get_build_dir(build_dir, CE_ARRAY_LEN(build_dir), application_platform());
    resource_compiler_get_tmp_dir(tmp_dir, CE_ARRAY_LEN(tmp_dir), application_platform());

    //////// VS
    yaml_as_string(vs_input, input_str, CE_ARRAY_LEN(input_str));
    compilator_api->add_dependency(filename, input_str);
    os_path_join(input_path, CE_ARRAY_LEN(input_path), source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CE_ARRAY_LEN(tmp_filename), input_str);

    int result = _shaderc(input_path, output_path, include_dir, "vertex", platform, vs_profile);

    if (result != 0) {
        return 0;
    }

    struct vio *tmp_file = vio_from_file(output_path, VIO_OPEN_READ, memsys_main_allocator());
    char *vs_data = CE_ALLOCATE(memsys_main_allocator(), char, vio_size(tmp_file) + 1);
    vio_read(tmp_file, vs_data, sizeof(char), vio_size(tmp_file));
    resource.vs_size = vio_size(tmp_file);
    vio_close(tmp_file);
    ///////

    //////// FS
    yaml_as_string(fs_input, input_str, CE_ARRAY_LEN(input_str));
    compilator_api->add_dependency(filename, input_str);
    os_path_join(input_path, CE_ARRAY_LEN(input_path), source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CE_ARRAY_LEN(tmp_filename), input_str);

    result = _shaderc(input_path, output_path, include_dir, "fragment", platform, fs_profile);

    if (result != 0) {
        return 0;
    }

    tmp_file = vio_from_file(output_path, VIO_OPEN_READ, memsys_main_allocator());
    char *fs_data = CE_ALLOCATE(memsys_main_allocator(), char, vio_size(tmp_file) + 1);
    vio_read(tmp_file, fs_data, sizeof(char), vio_size(tmp_file));
    resource.fs_size = vio_size(tmp_file);
    vio_close(tmp_file);

    vio_write(build_vio, &resource, sizeof(resource), 1);

    vio_write(build_vio, vs_data, sizeof(char), resource.vs_size);
    vio_write(build_vio, fs_data, sizeof(char), resource.fs_size);

    CE_DEALLOCATE(memsys_main_allocator(), vs_data);
    CE_DEALLOCATE(memsys_main_allocator(), fs_data);

    return 1;
}

//==============================================================================
// Resource
//==============================================================================

void *shader_resource_loader(struct vio *input,
                             struct allocator *allocator) {
    const i64 size = vio_size(input);
    char *data = CE_ALLOCATE(allocator, char, size);
    vio_read(input, data, 1, size);

    return data;
}

void shader_resource_unloader(void *new_data,
                              struct allocator *allocator) {
    CE_DEALLOCATE(allocator, new_data);
}

void shader_resource_online(stringid64_t name,
                            void *data) {
    struct shader_resource *resource = data;

    const bgfx_memory_t *vs_mem = bgfx_alloc(resource->vs_size);
    const bgfx_memory_t *fs_mem = bgfx_alloc(resource->fs_size);

    memory_copy(vs_mem->data, (resource + 1), resource->vs_size);
    memory_copy(fs_mem->data, ((char *) (resource + 1)) + resource->vs_size, resource->fs_size);

    bgfx_shader_handle_t vs_shader = bgfx_create_shader(vs_mem);
    bgfx_shader_handle_t fs_shader = bgfx_create_shader(fs_mem);

    bgfx_program_handle_t program = bgfx_create_program(vs_shader, fs_shader, 1);

    MAP_SET(bgfx_program_handle_t, &_G.handler_map, name.id, program);
}

static const bgfx_program_handle_t null_program = {0};

void shader_resource_offline(stringid64_t name,
                             void *data) {

    bgfx_program_handle_t program = MAP_GET(bgfx_program_handle_t, &_G.handler_map, name.id, null_program);

    if (program.idx == null_program.idx) {
        return;
    }

    bgfx_destroy_program(program);

    MAP_REMOVE(bgfx_program_handle_t, &_G.handler_map, name.id);
}

void *shader_resource_reloader(stringid64_t name,
                               void *old_data,
                               void *new_data,
                               struct allocator *allocator) {
    shader_resource_offline(name, old_data);
    shader_resource_online(name, new_data);

    CE_DEALLOCATE(allocator, old_data);

    return new_data;
}

static const resource_callbacks_t shader_resource_callback = {
        .loader = shader_resource_loader,
        .unloader =shader_resource_unloader,
        .online =shader_resource_online,
        .offline =shader_resource_offline,
        .reloader = shader_resource_reloader
};


//==============================================================================
// Interface
//==============================================================================

int shader_resource_init() {
    _G = (struct G) {0};

    _G.shader_type = stringid64_from_string("shader");

    MAP_INIT(bgfx_program_handle_t, &_G.handler_map, memsys_main_allocator());

    resource_compiler_register(_G.shader_type, _shader_resource_compiler);
    resource_register_type(_G.shader_type, shader_resource_callback);

    return 1;
}

void shader_resource_shutdown() {
    MAP_DESTROY(bgfx_program_handle_t, &_G.handler_map);
    _G = (struct G) {0};
}

bgfx_program_handle_t shader_resource_get(stringid64_t name) {
    return MAP_GET(bgfx_program_handle_t, &_G.handler_map, name.id, null_program);
}