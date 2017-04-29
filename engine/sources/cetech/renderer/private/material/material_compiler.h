#ifndef CETECH_MATERIAL_COMPILER_H
#define CETECH_MATERIAL_COMPILER_H

#include <cetech/os/path.h>

struct material_compile_output {
    ARRAY_T(char) uniform_names;
    ARRAY_T(uint8_t) data;
    uint32_t texture_count;
    uint32_t cel_vec4f_count;
    uint32_t mat33f_count;
    uint32_t cel_mat44f_count;
};

static void _preprocess(const char *filename,
                        yaml_node_t root,
                        struct compilator_api *capi) {
    yaml_node_t parent_node = yaml_get_node(root, "parent");

    if (yaml_is_valid(parent_node)) {
        char prefab_file[256] = {0};
        char prefab_str[256] = {0};
        yaml_as_string(parent_node, prefab_str, CEL_ARRAY_LEN(prefab_str));
        snprintf(prefab_file, CEL_ARRAY_LEN(prefab_file), "%s.material",
                 prefab_str);

        capi->add_dependency(filename, prefab_file);

        char full_path[256] = {0};
        const char *source_dir = ResourceApiV0.compiler_get_source_dir();
        cel_path_join(full_path, CEL_ARRAY_LEN(full_path), source_dir,
                      prefab_file);

        struct vio *prefab_vio = cel_vio_from_file(full_path, VIO_OPEN_READ,
                                                   MemSysApiV0.main_allocator());

        char prefab_data[cel_vio_size(prefab_vio) + 1];
        memory_set(prefab_data, 0, cel_vio_size(prefab_vio) + 1);
        cel_vio_read(prefab_vio, prefab_data, sizeof(char),
                     cel_vio_size(prefab_vio));
        cel_vio_close(prefab_vio);

        yaml_document_t h;
        yaml_node_t prefab_root = yaml_load_str(prefab_data, &h);

        _preprocess(filename, prefab_root, capi);
        yaml_merge(root, prefab_root);
    }
}

static void _forach_texture_clb(yaml_node_t key,
                                yaml_node_t value,
                                void *_data) {
    struct material_compile_output *output = _data;

    output->texture_count += 1;

    char tmp_buffer[512] = {0};
    char uniform_name[32] = {0};

    yaml_as_string(key, uniform_name, CEL_ARRAY_LEN(uniform_name) - 1);

    yaml_as_string(value, tmp_buffer, CEL_ARRAY_LEN(tmp_buffer));
    stringid64_t texture_name = stringid64_from_string(tmp_buffer);

    ARRAY_PUSH(char, &output->uniform_names, uniform_name,
               CEL_ARRAY_LEN(uniform_name));
    ARRAY_PUSH(uint8_t, &output->data, (uint8_t *) &texture_name, sizeof(stringid64_t));
}

static void _forach_vec4fs_clb(yaml_node_t key,
                               yaml_node_t value,
                               void *_data) {
    struct material_compile_output *output = _data;

    output->cel_vec4f_count += 1;

    char uniform_name[32] = {0};
    yaml_as_string(key, uniform_name, CEL_ARRAY_LEN(uniform_name) - 1);

    cel_vec4f_t v = yaml_as_cel_vec4f_t(value);

    ARRAY_PUSH(char, &output->uniform_names, uniform_name,
               CEL_ARRAY_LEN(uniform_name));
    ARRAY_PUSH(uint8_t, &output->data, (uint8_t *) &v, sizeof(cel_vec4f_t));
}

static void _forach_cel_mat44f_clb(yaml_node_t key,
                                   yaml_node_t value,
                                   void *_data) {
    struct material_compile_output *output = _data;

    output->cel_mat44f_count += 1;

    char uniform_name[32] = {0};
    yaml_as_string(key, uniform_name, CEL_ARRAY_LEN(uniform_name) - 1);

    cel_mat44f_t m = yaml_as_cel_mat44f_t(value);

    ARRAY_PUSH(char, &output->uniform_names, uniform_name,
               CEL_ARRAY_LEN(uniform_name));
    ARRAY_PUSH(uint8_t, &output->data, (uint8_t *) &m, sizeof(cel_mat44f_t));
}

static void _forach_mat33f_clb(yaml_node_t key,
                               yaml_node_t value,
                               void *_data) {
    struct material_compile_output *output = _data;

    output->mat33f_count += 1;

    char uniform_name[32] = {0};
    yaml_as_string(key, uniform_name, CEL_ARRAY_LEN(uniform_name) - 1);

    mat33f_t m = yaml_as_mat33f_t(value);

    ARRAY_PUSH(char, &output->uniform_names, uniform_name,
               CEL_ARRAY_LEN(uniform_name));
    ARRAY_PUSH(uint8_t, &output->data, (uint8_t *) &m, sizeof(mat33f_t));
}

static int _material_resource_compiler(const char *filename,
                                       struct vio *source_vio,
                                       struct vio *build_vio,
                                       struct compilator_api *compilator_api) {
    char *source_data =
    CEL_ALLOCATE(MemSysApiV0.main_allocator(), char,
                 cel_vio_size(source_vio) + 1);
    memory_set(source_data, 0, cel_vio_size(source_vio) + 1);

    cel_vio_read(source_vio, source_data, sizeof(char),
                 cel_vio_size(source_vio));

    yaml_document_t h;
    yaml_node_t root = yaml_load_str(source_data, &h);

    _preprocess(filename, root, compilator_api);

    yaml_node_t shader_node = yaml_get_node(root, "shader");
    CEL_ASSERT("material", yaml_is_valid(shader_node));

    char tmp_buffer[256] = {0};
    yaml_as_string(shader_node, tmp_buffer, CEL_ARRAY_LEN(tmp_buffer));

    struct material_compile_output output = {0};
    ARRAY_INIT(char, &output.uniform_names, MemSysApiV0.main_allocator());
    ARRAY_INIT(uint8_t, &output.data, MemSysApiV0.main_allocator());

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
        yaml_node_foreach_dict(mat44, _forach_cel_mat44f_clb, &output);
    }

    yaml_node_t mat33 = yaml_get_node(root, "mat33f");
    if (yaml_is_valid(mat33)) {
        yaml_node_foreach_dict(mat33, _forach_mat33f_clb, &output);
    }


    struct material_blob resource = {
            .shader_name = stringid64_from_string(tmp_buffer),
            .texture_count =output.texture_count,
            .vec4f_count = output.cel_vec4f_count,
            .uniforms_count = ARRAY_SIZE(&output.uniform_names) / 32,
    };

    cel_vio_write(build_vio, &resource, sizeof(resource), 1);
    cel_vio_write(build_vio, output.uniform_names.data, sizeof(char),
                  ARRAY_SIZE(&output.uniform_names));
    cel_vio_write(build_vio, output.data.data, sizeof(uint8_t),
                  ARRAY_SIZE(&output.data));

    ARRAY_DESTROY(char, &output.uniform_names);
    ARRAY_DESTROY(uint8_t, &output.data);
    CEL_DEALLOCATE(MemSysApiV0.main_allocator(), source_data);
    return 1;
}

#endif //CETECH_MATERIAL_COMPILER_H
