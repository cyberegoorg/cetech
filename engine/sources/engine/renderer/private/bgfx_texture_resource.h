#ifndef CETECH_BGFX_TEXTURE_RESOURCE_H
#define CETECH_BGFX_TEXTURE_RESOURCE_H

#include <celib/os/vio.h>
#include <celib/memory/memory.h>
#include <celib/yaml/yaml.h>
#include <celib/containers/array.h>
#include <engine/memory_system/memory_system.h>
#include <celib/os/process.h>
#include <engine/config_system/config_system.h>
#include <celib/os/path.h>
#include <engine/application/application.h>
#include <celib/os/fs.h>


int _texturec(const char *input, const char *output) {
    char cmd_line[4096] = {0};
    int s = snprintf(cmd_line, CE_ARRAY_LEN(cmd_line),
                     "externals/build/linux64/release/bin/texturec -f %s -o %s", input, output);

    int status = os_exec(cmd_line);

    log_info("application", "STATUS %d", status);

    return status;
}

int _tmp_filename(char *tmp_filename, const char *tmp_dir, size_t max_len, const char *filename) {
    char dir[1024] = {0};
    os_path_dir(dir, CE_ARRAY_LEN(dir), filename);

    os_path_join(tmp_filename, max_len, tmp_dir, dir);
    os_dir_make_path(tmp_filename);

    return snprintf(tmp_filename, max_len, "%s/%s.ktx", tmp_dir, filename);
}

int _texture_compiler(const char *filename,
                      struct vio *source_vio,
                      struct vio *build_vio,
                      struct compilator_api *compilator_api) {

    char source_data[vio_size(source_vio) + 1];
    memory_set(source_data, 0, vio_size(source_vio) + 1);
    vio_read(source_vio, source_data, sizeof(char), vio_size(source_vio));

    yaml_handler_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

//    input: content/texture1.tga
//    gen_mipmaps: true
//

    yaml_node_t input = yaml_get_node(h, root, "input");

    char build_dir[1024] = {0};
    char tmp_dir[1024] = {0};
    const char *source_dir = resource_compiler_get_source_dir();
    resource_compiler_get_build_dir(build_dir, CE_ARRAY_LEN(build_dir), application_platform());
    resource_compiler_get_tmp_dir(tmp_dir, CE_ARRAY_LEN(tmp_dir), application_platform());

    char input_str[1024] = {0};
    yaml_node_as_string(h, input, input_str, CE_ARRAY_LEN(input_str));

    char input_path[1024] = {0};
    char output_path[1024] = {0};

    char tmp_filename[1024] = {0};
    os_path_join(input_path, CE_ARRAY_LEN(input_path), source_dir, input_str);

    _tmp_filename(output_path, tmp_dir, CE_ARRAY_LEN(tmp_filename), input_str);

    int resutl = _texturec(input_path, output_path);

    if (resutl != 0) {
        return 0;
    }

    struct vio *tmp_file = vio_from_file(output_path, VIO_OPEN_READ, memsys_main_allocator());
    char *tmp_data = CE_ALLOCATE(memsys_main_allocator(), char, vio_size(tmp_file) + 1);
    vio_read(tmp_file, tmp_data, sizeof(char), vio_size(tmp_file));
    vio_write(build_vio, tmp_data, sizeof(char), vio_size(tmp_file));
    vio_close(tmp_file);
    CE_DEALLOCATE(memsys_main_allocator(), tmp_data);

    return 1;
}

#endif //CETECH_BGFX_TEXTURE_RESOURCE_H
