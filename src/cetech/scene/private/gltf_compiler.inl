#define CGLTF_IMPLEMENTATION

#include "include/cgltf/cgltf.h"

static inline bgfx_attrib_t _to_bgxfx_attr(cgltf_attribute_type attr) {
    switch (attr) {
        case cgltf_attribute_type_invalid:
            break;

        case cgltf_attribute_type_position:
            return BGFX_ATTRIB_POSITION;

        case cgltf_attribute_type_normal:
            return BGFX_ATTRIB_NORMAL;

        case cgltf_attribute_type_tangent:
            return BGFX_ATTRIB_TANGENT;

        case cgltf_attribute_type_texcoord:
            return BGFX_ATTRIB_TEXCOORD0;

        case cgltf_attribute_type_color:
            return BGFX_ATTRIB_COLOR0;

        case cgltf_attribute_type_joints:
            break;

        case cgltf_attribute_type_weights:
            return BGFX_ATTRIB_WEIGHT;
    }

    return BGFX_ATTRIB_COUNT;
}

static inline bgfx_attrib_type_t _to_bgfx_attr_type(cgltf_component_type type) {
    switch (type) {
        case cgltf_component_type_invalid:
            break;

        case cgltf_component_type_r_8:
            break;

        case cgltf_component_type_r_8u:
            return BGFX_ATTRIB_TYPE_UINT8;

        case cgltf_component_type_r_16:
            return BGFX_ATTRIB_TYPE_INT16;

        case cgltf_component_type_r_16u:
            break;

        case cgltf_component_type_r_32u:
            break;

        case cgltf_component_type_r_32f:
            return BGFX_ATTRIB_TYPE_FLOAT;

    }

    return BGFX_ATTRIB_TYPE_COUNT;
}

static inline int _comp_size(bgfx_attrib_type_t type) {
    switch (type) {
        case BGFX_ATTRIB_TYPE_UINT8:
            return sizeof(uint8_t);
        case BGFX_ATTRIB_TYPE_UINT10:
            break;
        case BGFX_ATTRIB_TYPE_INT16:
            return sizeof(uint16_t);
        case BGFX_ATTRIB_TYPE_HALF:
            break;
        case BGFX_ATTRIB_TYPE_FLOAT:
            return sizeof(float);
        case BGFX_ATTRIB_TYPE_COUNT:
            break;
    }

    return 0;
}

static int _gltf_comp_num(cgltf_type type) {
    switch (type) {
        default:
        case cgltf_type_invalid:
            return 0;

        case cgltf_type_scalar:
            return 1;

        case cgltf_type_vec2:
            return 2;

        case cgltf_type_vec3:
            return 3;

        case cgltf_type_vec4:
            return 4;

        case cgltf_type_mat2:
            return 2 * 2;

        case cgltf_type_mat3:
            return 3 * 3;

        case cgltf_type_mat4:
            return 4 * 4;
    }
}

static bool _compile_gtlf(ce_cdb_t0 db,
                          uint64_t k,
                          scene_compile_output_t *output) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, k);
    uint64_t import_obj = ce_cdb_a0->read_subobject(reader, SCENE_IMPORT_PROP, 0);

    ct_scene_import_obj_t0 io = {};
    ce_cdb_a0->read_to(db, import_obj, &io, sizeof(io));

    const ce_cdb_obj_o0 *c_reader = ce_cdb_a0->read(ce_cdb_a0->db(), ce_config_a0->obj());
    const char *source_dir = ce_cdb_a0->read_str(c_reader, CONFIG_SRC, "");

    char *input_path = NULL;
    ce_os_path_a0->join(&input_path, _G.allocator, 2, source_dir, io.input);

    cgltf_options options = {};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, input_path, &data);
    if (result != cgltf_result_success) {
        return false;
    }

    result = cgltf_load_buffers(&options, data, input_path);
    if (result != cgltf_result_success) {
        return false;
    }

    for (int m = 0; m < data->meshes_count; ++m) {
        cgltf_mesh *mesh = &data->meshes[m];

        size_t name_len = strlen(mesh->name);

        char tmp_buffer[1024] = {};
        char tmp_buffer2[1024] = {};
        uint32_t unique = 0;

        if (name_len == 0) {
            snprintf(tmp_buffer, CE_ARRAY_LEN(tmp_buffer), "geom_%d", m);
        } else {
            memcpy(tmp_buffer, mesh->name, name_len);
        }

        uint64_t name_id = ce_id_a0->id64(tmp_buffer);
        for (uint32_t i = 0; k < ce_array_size(output->geom_name); ++i) {
            if (name_id == output->geom_name[k]) {
                snprintf(tmp_buffer2, CE_ARRAY_LEN(tmp_buffer2), "%s%d", tmp_buffer, ++unique);
                snprintf(tmp_buffer, CE_ARRAY_LEN(tmp_buffer), "%s", tmp_buffer2);
                break;
            }
        }

        char tmp_name[128] = {};
        strncpy(tmp_name, tmp_buffer, 127);
        ce_array_push_n(output->geom_str, &tmp_name, 1, _G.allocator);
        ce_array_push(output->geom_name, ce_id_a0->id64(tmp_buffer), _G.allocator);
        ce_array_push(output->ib_offset, ce_array_size(output->ib), _G.allocator);
        ce_array_push(output->vb_offset, ce_array_size(output->vb), _G.allocator);

        bgfx_vertex_decl_t vertex_decl;
        ct_gfx_a0->bgfx_vertex_decl_begin(&vertex_decl, BGFX_RENDERER_TYPE_COUNT);

        uint32_t v_size = 0;
        uint32_t vertex_n = 0;
        cgltf_primitive *prim = &mesh->primitives[0];
        for (int l = 0; l < prim->attributes_count; ++l) {
            cgltf_attribute *attr = &prim->attributes[l];
            cgltf_accessor *acess = attr->data;

            vertex_n = acess->count;

            bgfx_attrib_t bgfx_attr = _to_bgxfx_attr(attr->type);
            bgfx_attrib_type_t bgfx_attr_type = _to_bgfx_attr_type(acess->component_type);

            int num = _gltf_comp_num(acess->type);
            bool normalized = bgfx_attr == BGFX_ATTRIB_NORMAL;

            ct_gfx_a0->bgfx_vertex_decl_add(&vertex_decl,
                                            bgfx_attr, num,
                                            bgfx_attr_type, normalized, 0);

            v_size += _comp_size(bgfx_attr_type) * num;
        }

        ct_gfx_a0->bgfx_vertex_decl_end(&vertex_decl);

        uint32_t indices_n = prim->indices->count;

        ce_array_push(output->vb_decl, vertex_decl, _G.allocator);
        ce_array_push(output->vb_size, v_size * vertex_n, _G.allocator);
        ce_array_push(output->ib_size, indices_n, _G.allocator);

        for (int j = 0; j < vertex_n; ++j) {
            for (int l = 0; l < prim->attributes_count; ++l) {
                cgltf_attribute *attr = &prim->attributes[l];

                uint32_t c_n = cgltf_num_components(attr->data->type);

                float f[c_n];
                cgltf_accessor_read_float(attr->data, j, f, c_n);

                ce_array_push_n(output->vb, (uint8_t *) f, sizeof(float) * c_n, _G.allocator);
            }
        }

        for (int n = 0; n < indices_n; ++n) {
            uint32_t idx = cgltf_accessor_read_index(prim->indices, n);
            ce_array_push(output->ib, idx, _G.allocator);
        }

    }

    cgltf_free(data);
    return true;
}
