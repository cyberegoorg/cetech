#ifndef CETECH_SHADER_COMPILER_H
#define CETECH_SHADER_COMPILER_H


#include <cetech/core/fs.h>
#include <cetech/core/os.h>
#include <cetech/core/yaml.h>
#include <stdio.h>

static int _shaderc(const char *input,
                    const char *output,
                    const char *include_path,
                    const char *type,
                    const char *platform,
                    const char *profile) {
    char cmd_line[4096] = {0};

    int s = resource_api_v0.compiler_external_join(cmd_line,
                                                   CETECH_ARRAY_LEN(cmd_line),
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

    int status = exec(cmd_line);

    log_debug("shaderc", "STATUS %d", status);

    return status;
}

static int _gen_tmp_name(char *tmp_filename,
                         const char *tmp_dir,
                         size_t max_len,
                         const char *filename) {
    char dir[1024] = {0};
    path_dir(dir, CETECH_ARRAY_LEN(dir), filename);

    path_join(tmp_filename, max_len, tmp_dir, dir);
    dir_make_path(tmp_filename);

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

static int _shader_resource_compiler(const char *filename,
                                     struct vio *source_vio,
                                     struct vio *build_vio,
                                     struct compilator_api *compilator_api) {

    char source_data[vio_size(source_vio) + 1];
    memory_set(source_data, 0, vio_size(source_vio) + 1);
    vio_read(source_vio, source_data, sizeof(char),
             vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t vs_input = yaml_get_node(root, "vs_input");
    yaml_node_t fs_input = yaml_get_node(root, "fs_input");

    const char *source_dir = resource_api_v0.compiler_get_source_dir();
    const char *core_dir = resource_api_v0.compiler_get_core_dir();

    char include_dir[1024] = {0};
    path_join(include_dir, CETECH_ARRAY_LEN(include_dir), core_dir,
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
    path_join(input_path, CETECH_ARRAY_LEN(input_path), source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                  input_str);

    int result = _shaderc(input_path, output_path, include_dir, "vertex",
                          platform, vs_profile);

    if (result != 0) {
        return 0;
    }

    struct vio *tmp_file = vio_from_file(output_path, VIO_OPEN_READ,
                                         memory_api_v0.main_allocator());
    char *vs_data =
            CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                            vio_size(tmp_file) + 1);
    vio_read(tmp_file, vs_data, sizeof(char), vio_size(tmp_file));
    resource.vs_size = vio_size(tmp_file);
    vio_close(tmp_file);
    ///////

    //////// FS
    yaml_as_string(fs_input, input_str, CETECH_ARRAY_LEN(input_str));
    compilator_api->add_dependency(filename, input_str);
    path_join(input_path, CETECH_ARRAY_LEN(input_path), source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                  input_str);

    result = _shaderc(input_path, output_path, include_dir, "fragment",
                      platform, fs_profile);

    if (result != 0) {
        return 0;
    }

    tmp_file = vio_from_file(output_path, VIO_OPEN_READ,
                             memory_api_v0.main_allocator());
    char *fs_data =
            CETECH_ALLOCATE(memory_api_v0.main_allocator(), char,
                            vio_size(tmp_file) + 1);
    vio_read(tmp_file, fs_data, sizeof(char), vio_size(tmp_file));
    resource.fs_size = vio_size(tmp_file);
    vio_close(tmp_file);

    vio_write(build_vio, &resource, sizeof(resource), 1);

    vio_write(build_vio, vs_data, sizeof(char), resource.vs_size);
    vio_write(build_vio, fs_data, sizeof(char), resource.fs_size);

    CETECH_DEALLOCATE(memory_api_v0.main_allocator(), vs_data);
    CETECH_DEALLOCATE(memory_api_v0.main_allocator(), fs_data);

    return 1;
}


#endif //CETECH_SHADER_COMPILER_H
