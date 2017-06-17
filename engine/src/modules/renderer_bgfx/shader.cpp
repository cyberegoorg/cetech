//==============================================================================
// Include
//==============================================================================

#include <cstdio>
#include <bgfx/c99/bgfx.h>

#include <cetech/celib/allocator.h>
#include <cetech/celib/map.inl>

#include <cetech/core/hash.h>
#include <cetech/core/application.h>
#include <cetech/core/memory.h>
#include <cetech/core/path.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/core/log.h>
#include <cetech/core/vio.h>
#include <cetech/core/yaml.h>
#include <cetech/core/process.h>

#include <cetech/modules/resource.h>

using namespace cetech;

//==============================================================================
// Structs
//==============================================================================

struct shader {
    uint64_t vs_size;
    uint64_t fs_size;
    // uint8_t vs [vs_size]
    // uint8_t fs [fs_size]
};

//==============================================================================
// GLobals
//==============================================================================

#define _G ShaderResourceGlobals
struct ShaderResourceGlobals {
    Map<bgfx_program_handle_t> handler_map;
    uint64_t type;
} ShaderResourceGlobals;


IMPORT_API(memory_api_v0)
IMPORT_API(resource_api_v0)
IMPORT_API(app_api_v0)
IMPORT_API(path_v0)
IMPORT_API(vio_api_v0)
IMPORT_API(process_api_v0)
IMPORT_API(log_api_v0)
IMPORT_API(hash_api_v0)

//==============================================================================
// Compiler private
//==============================================================================
#ifdef CETECH_CAN_COMPILE

namespace shader_resource_compiler {

    static int _shaderc(const char *input,
                        const char *output,
                        const char *include_path,
                        const char *type,
                        const char *platform,
                        const char *profile) {
        char cmd_line[4096] = {0};

        int s = resource_api_v0.compiler_external_join(cmd_line,
                                                       CETECH_ARRAY_LEN(
                                                               cmd_line),
                                                       "shaderc");
        s += snprintf(cmd_line + s, CETECH_ARRAY_LEN(cmd_line) - s,
                      ""
                              " -f %s"
                              " -o %s"
                              " -i %s"
                              " --type %s"
                              " --platform %s"
                              " --profile %s"

                              " 2>&1",  // TODO: move to exec

                      input, output, include_path, type, platform, profile);

        int status = process_api_v0.exec(cmd_line);

        log_api_v0.debug("shaderc", "STATUS %d", status);

        return status;
    }

    static int _gen_tmp_name(char *tmp_filename,
                             const char *tmp_dir,
                             size_t max_len,
                             const char *filename) {
        char dir[1024] = {0};
        path_v0.dir(dir, CETECH_ARRAY_LEN(dir), filename);

        path_v0.join(tmp_filename, max_len, tmp_dir, dir);
        path_v0.make_path(tmp_filename);

        return snprintf(tmp_filename, max_len, "%s/%s.shaderc", tmp_dir,
                        filename);
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

    static int compiler(const char *filename,
                        struct vio *source_vio,
                        struct vio *build_vio,
                        struct compilator_api *compilator_api) {

        char source_data[vio_api_v0.size(source_vio) + 1];
        memset(source_data, 0, vio_api_v0.size(source_vio) + 1);
        vio_api_v0.read(source_vio, source_data, sizeof(char),
                        vio_api_v0.size(source_vio));

        yaml_document_t h;
        yaml_node_t root = yaml_load_str(source_data, &h);

        yaml_node_t vs_input = yaml_get_node(root, "vs_input");
        yaml_node_t fs_input = yaml_get_node(root, "fs_input");

        const char *source_dir = resource_api_v0.compiler_get_source_dir();
        const char *core_dir = resource_api_v0.compiler_get_core_dir();

        char include_dir[1024] = {0};
        path_v0.join(include_dir, CETECH_ARRAY_LEN(include_dir), core_dir,
                     "bgfxshaders");

        struct shader resource = {0};

        // TODO: temp allocator?
        char build_dir[4096] = {0};
        char tmp_dir[4096] = {0};
        char input_str[1024] = {0};
        char input_path[1024] = {0};
        char output_path[4096] = {0};
        char tmp_filename[4096] = {0};

        resource_api_v0.compiler_get_build_dir(build_dir,
                                               CETECH_ARRAY_LEN(build_dir),
                                               app_api_v0.platform());
        resource_api_v0.compiler_get_tmp_dir(tmp_dir, CETECH_ARRAY_LEN(tmp_dir),
                                             app_api_v0.platform());

        //////// VS
        yaml_as_string(vs_input, input_str, CETECH_ARRAY_LEN(input_str));
        compilator_api->add_dependency(filename, input_str);
        path_v0.join(input_path, CETECH_ARRAY_LEN(input_path), source_dir,
                     input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        int result = _shaderc(input_path, output_path, include_dir, "vertex",
                              platform, vs_profile);

        if (result != 0) {
            return 0;
        }

        struct vio *tmp_file = vio_api_v0.from_file(output_path, VIO_OPEN_READ,
                                                    memory_api_v0.main_allocator());
        char *vs_data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                vio_api_v0.size(tmp_file) + 1);
        vio_api_v0.read(tmp_file, vs_data, sizeof(char),
                        vio_api_v0.size(tmp_file));
        resource.vs_size = vio_api_v0.size(tmp_file);
        vio_api_v0.close(tmp_file);
        ///////

        //////// FS
        yaml_as_string(fs_input, input_str, CETECH_ARRAY_LEN(input_str));
        compilator_api->add_dependency(filename, input_str);
        path_v0.join(input_path, CETECH_ARRAY_LEN(input_path), source_dir,
                     input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        result = _shaderc(input_path, output_path, include_dir, "fragment",
                          platform, fs_profile);

        if (result != 0) {
            return 0;
        }

        tmp_file = vio_api_v0.from_file(output_path, VIO_OPEN_READ,
                                        memory_api_v0.main_allocator());
        char *fs_data =
                CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                                vio_api_v0.size(tmp_file) + 1);
        vio_api_v0.read(tmp_file, fs_data, sizeof(char),
                        vio_api_v0.size(tmp_file));
        resource.fs_size = vio_api_v0.size(tmp_file);
        vio_api_v0.close(tmp_file);

        vio_api_v0.write(build_vio, &resource, sizeof(resource), 1);

        vio_api_v0.write(build_vio, vs_data, sizeof(char), resource.vs_size);
        vio_api_v0.write(build_vio, fs_data, sizeof(char), resource.fs_size);

        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), vs_data);
        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), fs_data);

        return 1;
    }

}

#endif

//==============================================================================
// Resource
//==============================================================================

namespace shader_resource {

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
        struct shader *resource = (shader *) data;

        const bgfx_memory_t *vs_mem = bgfx_alloc(resource->vs_size);
        const bgfx_memory_t *fs_mem = bgfx_alloc(resource->fs_size);

        memcpy(vs_mem->data, (resource + 1), resource->vs_size);
        memcpy(fs_mem->data, ((char *) (resource + 1)) + resource->vs_size,
               resource->fs_size);

        bgfx_shader_handle_t vs_shader = bgfx_create_shader(vs_mem);
        bgfx_shader_handle_t fs_shader = bgfx_create_shader(fs_mem);

        bgfx_program_handle_t program = bgfx_create_program(vs_shader,
                                                            fs_shader,
                                                            1);

        map::set(_G.handler_map, name, program);
    }

    void offline(uint64_t name,
                 void *data) {

        bgfx_program_handle_t program = map::get(_G.handler_map, name,
                                                 null_program);

        if (program.idx == null_program.idx) {
            return;
        }

        bgfx_destroy_program(program);

        map::remove(_G.handler_map, name);
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

    static const resource_callbacks_t callback = {
            .loader = loader,
            .unloader =unloader,
            .online = online,
            .offline = offline,
            .reloader = reloader
    };

}

//==============================================================================
// Interface
//==============================================================================

int shader_init(struct api_v0 *api) {

    GET_API(api, memory_api_v0);
    GET_API(api, resource_api_v0);
    GET_API(api, app_api_v0);
    GET_API(api, path_v0);
    GET_API(api, vio_api_v0);
    GET_API(api, process_api_v0);
    GET_API(api, log_api_v0);
    GET_API(api, hash_api_v0);

    _G = {0};

    _G.type = hash_api_v0.id64_from_str("shader");

    _G.handler_map.init(memory_api_v0.main_allocator());

    resource_api_v0.register_type(_G.type,
                                  shader_resource::callback);
#ifdef CETECH_CAN_COMPILE
    resource_api_v0.compiler_register(_G.type,
                                      shader_resource_compiler::compiler);
#endif
    return 1;
}

void shader_shutdown() {
    _G.handler_map.destroy();
}

bgfx_program_handle_t shader_get(uint64_t name) {
    struct shader *resource = (shader *) resource_api_v0.get(_G.type, name);
    return map::get(_G.handler_map, name, shader_resource::null_program);
}