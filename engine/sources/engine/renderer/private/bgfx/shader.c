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

#include "engine/application/application.h"
#include "engine/resource/resource.h"
#include <engine/memory/memsys.h>
#include <engine/plugin/plugin.h>
#include "engine/memory/memsys.h"


//==============================================================================
// Structs
//==============================================================================

ARRAY_PROTOTYPE(bgfx_program_handle_t)

MAP_PROTOTYPE(bgfx_program_handle_t)

struct shader {
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
    stringid64_t type;
} _G = {0};

static struct MemSysApiV1 MemSysApiV1;
static struct ResourceApiV1 ResourceApiV1;

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

    int s = ResourceApiV1.compiler_external_join(cmd_line, CEL_ARRAY_LEN(cmd_line), "shaderc");
    s += snprintf(cmd_line + s, CEL_ARRAY_LEN(cmd_line) - s,
                  ""
                          " -f %s"
                          " -o %s"
                          " -i %s"
                          " --type %s"
                          " --platform %s"
                          " --profile %s"

                          " 2>&1",  // TODO: move to cel_exec

                  input, output, include_path, type, platform, profile);

    int status = cel_exec(cmd_line);

    log_debug("shaderc", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {
    char dir[1024] = {0};
    cel_path_dir(dir, CEL_ARRAY_LEN(dir), filename);

    cel_path_join(tmp_filename, max_len, tmp_dir, dir);
    cel_dir_make_path(tmp_filename);

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

    char source_data[cel_vio_size(source_vio) + 1];
    memory_set(source_data, 0, cel_vio_size(source_vio) + 1);
    cel_vio_read(source_vio, source_data, sizeof(char), cel_vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t vs_input = yaml_get_node(root, "vs_input");
    yaml_node_t fs_input = yaml_get_node(root, "fs_input");

    const char *source_dir = ResourceApiV1.compiler_get_source_dir();
    const char *core_dir = ResourceApiV1.compiler_get_core_dir();

    char include_dir[1024] = {0};
    cel_path_join(include_dir, CEL_ARRAY_LEN(include_dir), core_dir, "bgfxshaders");

    struct shader resource = {0};

    // TODO: temp allocator?
    char build_dir[4096] = {0};
    char tmp_dir[4096] = {0};
    char input_str[1024] = {0};
    char input_path[1024] = {0};
    char output_path[4096] = {0};
    char tmp_filename[4096] = {0};

    ResourceApiV1.compiler_get_build_dir(build_dir, CEL_ARRAY_LEN(build_dir), application_platform());
    ResourceApiV1.compiler_get_tmp_dir(tmp_dir, CEL_ARRAY_LEN(tmp_dir), application_platform());

    //////// VS
    yaml_as_string(vs_input, input_str, CEL_ARRAY_LEN(input_str));
    compilator_api->add_dependency(filename, input_str);
    cel_path_join(input_path, CEL_ARRAY_LEN(input_path), source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CEL_ARRAY_LEN(tmp_filename), input_str);

    int result = _shaderc(input_path, output_path, include_dir, "vertex", platform, vs_profile);

    if (result != 0) {
        return 0;
    }

    struct vio *tmp_file = cel_vio_from_file(output_path, VIO_OPEN_READ, MemSysApiV1.main_allocator());
    char *vs_data = CEL_ALLOCATE(MemSysApiV1.main_allocator(), char, cel_vio_size(tmp_file) + 1);
    cel_vio_read(tmp_file, vs_data, sizeof(char), cel_vio_size(tmp_file));
    resource.vs_size = cel_vio_size(tmp_file);
    cel_vio_close(tmp_file);
    ///////

    //////// FS
    yaml_as_string(fs_input, input_str, CEL_ARRAY_LEN(input_str));
    compilator_api->add_dependency(filename, input_str);
    cel_path_join(input_path, CEL_ARRAY_LEN(input_path), source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CEL_ARRAY_LEN(tmp_filename), input_str);

    result = _shaderc(input_path, output_path, include_dir, "fragment", platform, fs_profile);

    if (result != 0) {
        return 0;
    }

    tmp_file = cel_vio_from_file(output_path, VIO_OPEN_READ, MemSysApiV1.main_allocator());
    char *fs_data = CEL_ALLOCATE(MemSysApiV1.main_allocator(), char, cel_vio_size(tmp_file) + 1);
    cel_vio_read(tmp_file, fs_data, sizeof(char), cel_vio_size(tmp_file));
    resource.fs_size = cel_vio_size(tmp_file);
    cel_vio_close(tmp_file);

    cel_vio_write(build_vio, &resource, sizeof(resource), 1);

    cel_vio_write(build_vio, vs_data, sizeof(char), resource.vs_size);
    cel_vio_write(build_vio, fs_data, sizeof(char), resource.fs_size);

    CEL_DEALLOCATE(MemSysApiV1.main_allocator(), vs_data);
    CEL_DEALLOCATE(MemSysApiV1.main_allocator(), fs_data);

    return 1;
}

//==============================================================================
// Resource
//==============================================================================

void *shader_resource_loader(struct vio *input,
                             struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void shader_resource_unloader(void *new_data,
                              struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
}

void shader_resource_online(stringid64_t name,
                            void *data) {
    struct shader *resource = data;

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
                               struct cel_allocator *allocator) {
    shader_resource_offline(name, old_data);
    shader_resource_online(name, new_data);

    CEL_DEALLOCATE(allocator, old_data);

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

    MemSysApiV1 = *(struct MemSysApiV1*)plugin_get_engine_api(MEMORY_API_ID, 0);
    ResourceApiV1 = *(struct ResourceApiV1*)plugin_get_engine_api(RESOURCE_API_ID, 0);

    _G.type = stringid64_from_string("shader");

    MAP_INIT(bgfx_program_handle_t, &_G.handler_map, MemSysApiV1.main_allocator());

    ResourceApiV1.compiler_register(_G.type, _shader_resource_compiler);
    ResourceApiV1.register_type(_G.type, shader_resource_callback);

    return 1;
}

void shader_resource_shutdown() {
    MAP_DESTROY(bgfx_program_handle_t, &_G.handler_map);
    _G = (struct G) {0};
}

bgfx_program_handle_t shader_resource_get(stringid64_t name) {
    struct shader *resource = ResourceApiV1.get(_G.type, name);
    return MAP_GET(bgfx_program_handle_t, &_G.handler_map, name.id, null_program);
}