#ifndef CETECH_TEXTURE_COMPILER_H
#define CETECH_TEXTURE_COMPILER_H

#include <cetech/core/os/path.h>
#include <cetech/core/yaml.h>
#include <stdio.h>

static int _texturec(const char *input,
                     const char *output,
                     int gen_mipmaps,
                     int is_normalmap) {
    char cmd_line[4096] = {0};

    int s = resource_api_v0.compiler_external_join(cmd_line,
                                                   CETECH_ARRAY_LEN(cmd_line),
                                                   "texturec");

    s += snprintf(cmd_line + s, CETECH_ARRAY_LEN(cmd_line) - s, " -f %s -o %s",
                  input, output);
    if (gen_mipmaps) {
        s += snprintf(cmd_line + s, CETECH_ARRAY_LEN(cmd_line) - s, " --mips");
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

    int gen_mipmaps = yaml_is_valid(n_gen_mipmaps) ? yaml_as_bool(n_gen_mipmaps)
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

    int result = _texturec(input_path, output_path, gen_mipmaps, is_normalmap);

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

#endif //CETECH_TEXTURE_COMPILER_H
