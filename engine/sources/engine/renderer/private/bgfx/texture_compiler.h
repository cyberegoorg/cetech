#ifndef CETECH_TEXTURE_COMPILER_H
#define CETECH_TEXTURE_COMPILER_H


static int _texturec(const char *input,
                     const char *output,
                     int gen_mipmaps,
                     int is_normalmap) {
    char cmd_line[4096] = {0};

    int s = ResourceApiV0.compiler_external_join(cmd_line,
                                                 CEL_ARRAY_LEN(cmd_line),
                                                 "texturec");

    s += snprintf(cmd_line + s, CEL_ARRAY_LEN(cmd_line) - s, " -f %s -o %s",
                  input, output);
    if (gen_mipmaps) {
        s += snprintf(cmd_line + s, CEL_ARRAY_LEN(cmd_line) - s, " --mips");
    }

    if (is_normalmap) {
        s += snprintf(cmd_line + s, CEL_ARRAY_LEN(cmd_line) - s,
                      " --normalmap");
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

    char source_data[cel_vio_size(source_vio) + 1];
    memory_set(source_data, 0, cel_vio_size(source_vio) + 1);
    cel_vio_read(source_vio, source_data, sizeof(char),
                 cel_vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    yaml_node_t input = yaml_get_node(root, "input");
    yaml_node_t n_gen_mipmaps = yaml_get_node(root, "gen_mipmaps");
    yaml_node_t n_is_normalmap = yaml_get_node(root, "is_normalmap");

    int gen_mipmaps = yaml_is_valid(n_gen_mipmaps) ? yaml_as_bool(n_gen_mipmaps)
                                                   : 0;
    int is_normalmap = yaml_is_valid(n_is_normalmap) ? yaml_as_bool(
            n_is_normalmap) : 0;

    const char *source_dir = ResourceApiV0.compiler_get_source_dir();

    ResourceApiV0.compiler_get_build_dir(build_dir, CEL_ARRAY_LEN(build_dir),
                                         ApplicationApiV0.platform());
    ResourceApiV0.compiler_get_tmp_dir(tmp_dir, CEL_ARRAY_LEN(tmp_dir),
                                       ApplicationApiV0.platform());

    yaml_as_string(input, input_str, CEL_ARRAY_LEN(input_str));

    cel_path_join(input_path, CEL_ARRAY_LEN(input_path), source_dir, input_str);

    _gen_tmp_name(output_path, tmp_dir, CEL_ARRAY_LEN(tmp_filename), input_str);

    int result = _texturec(input_path, output_path, gen_mipmaps, is_normalmap);

    if (result != 0) {
        return 0;
    }

    struct vio *tmp_file = cel_vio_from_file(output_path, VIO_OPEN_READ,
                                             MemSysApiV0.main_allocator());
    char *tmp_data = CEL_ALLOCATE(MemSysApiV0.main_allocator(), char,
                                  cel_vio_size(tmp_file) + 1);
    cel_vio_read(tmp_file, tmp_data, sizeof(char), cel_vio_size(tmp_file));

    struct texture resource = {
            .size = cel_vio_size(tmp_file)
    };

    cel_vio_write(build_vio, &resource, sizeof(resource), 1);
    cel_vio_write(build_vio, tmp_data, sizeof(char), resource.size);

    cel_vio_close(tmp_file);
    CEL_DEALLOCATE(MemSysApiV0.main_allocator(), tmp_data);

    compilator_api->add_dependency(filename, input_str);

    return 1;
}


#endif //CETECH_TEXTURE_COMPILER_H
