#define CGLTF_IMPLEMENTATION

#include <celib/macros.h>
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
    static int tbl[] = {
            [cgltf_type_invalid] = 0,
            [cgltf_type_scalar] = 1,
            [cgltf_type_vec2] = 2,
            [cgltf_type_vec3] = 3,
            [cgltf_type_vec4] = 4,
            [cgltf_type_mat2] = 2 * 2,
            [cgltf_type_mat3] = 3 * 3,
            [cgltf_type_mat4] = 4 * 4,
    };
    return tbl[type];
}

static bool _compile_gtlf(ce_cdb_t0 db,
                          uint64_t k,
                          scene_compile_output_t *output) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(db, k);
    uint64_t import_obj = ce_cdb_a0->read_subobject(reader, SCENE_IMPORT_PROP, 0);

    ct_scene_import_obj_t0 io = {};
    ce_cdb_a0->read_to(db, import_obj, &io, sizeof(io));

    const char *source_dir = ce_config_a0->read_str(CONFIG_SRC, "");

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
    return true;
}

////
static bool _import_gtlf(ce_cdb_t0 db,
                         uint64_t dcc_obj) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, dcc_obj);
    const char *filename = ce_cdb_a0->read_str(r, CT_DCC_FILENAME_PROP, NULL);

    {
        uint32_t assets_n = ce_cdb_a0->read_objset_num(r, CT_DCC_RESOURCE_ASSETS_PROP);
        if (!assets_n) {
            char resource_path[512];
            char basae_name[128] = {};
            ce_os_path_a0->basename(basae_name, filename);
            char dir_name[128] = {};
            ce_os_path_a0->dirname(dir_name, filename);
            snprintf(resource_path, CE_ARRAY_LEN(resource_path), "%s/%s_resource", dir_name,
                     basae_name);

            ce_fs_a0->create_directory(SOURCE_ROOT, resource_path);


            const char *source_dir = ce_config_a0->read_str(CONFIG_SRC, "");

            char *input_path = NULL;
            ce_os_path_a0->join(&input_path, _G.allocator, 2, source_dir, filename);

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

            ce_hash_t mat_to_obj = {};
            for (int m = 0; m < data->materials_count; ++m) {
                cgltf_material *mat = &data->materials[m];

                uint64_t base_material = ce_cdb_a0->obj_from_uid(db,
                                                                 (ce_cdb_uuid_t0) {
                                                                         0x24c9413e88ebaaa8});
                uint64_t new_mat = ce_cdb_a0->create_from(db, base_material);
                ce_hash_add(&mat_to_obj, (uint64_t) mat, new_mat, _G.allocator);

                if (mat->pbr_metallic_roughness.base_color_texture.texture) {
                    const char *tname = mat->pbr_metallic_roughness.base_color_texture.texture->image->uri;

                    char dirname[512] = {};
                    ce_os_path_a0->dirname(dirname, filename);

                    char texture_file[512] = {};
                    snprintf(texture_file, CE_ARRAY_LEN(texture_file), "%s/%s.texture", dirname,
                             tname);

                    ce_cdb_uuid_t0 texture_uid = ct_resourcedb_a0->get_file_resource(texture_file);
                    uint64_t texture_obj = ce_cdb_a0->obj_from_uid(db, texture_uid);
                    ct_material_a0->set_texture(db, new_mat, _GBUFFER, "u_texColor", texture_obj);
                }

                char material_name[128] = {};
                if (mat->name) {
                    snprintf(material_name, CE_ARRAY_LEN(material_name), "%s", mat->name);
                } else {
                    snprintf(material_name, CE_ARRAY_LEN(material_name), "%d", m);
                }


                char material_filename[512] = {};
                snprintf(material_filename, CE_ARRAY_LEN(material_filename),
                         "%s/%s.material", resource_path, material_name);

                ct_asset_io_a0->save_to_cdb(db, new_mat, material_filename);
                ct_asset_io_a0->save_to_file(db, new_mat, material_filename);
            }

            scene_compile_output_t *output = _crete_compile_output();
            for (int m = 0; m < data->meshes_count; ++m) {
                cgltf_mesh *mesh = &data->meshes[m];
                _compile_gltf_mesh(mesh, m, output);
            }
            uint64_t scene_obj = ce_cdb_a0->create_object(db, SCENE_TYPE);
            _fill_scene(db, scene_obj, output);

            char scene_filename[512] = {};
            snprintf(scene_filename, CE_ARRAY_LEN(scene_filename), "%s.scene", filename);
            ct_asset_io_a0->save_to_cdb(db, scene_obj, scene_filename);

            _destroy_compile_output(output);

//            for (int m = 0; m < data->images_count; ++m) {
//                cgltf_image *image = &data->images[m];
//            }

            uint64_t root_ent = ce_cdb_a0->create_object(db, ENTITY_TYPE);
            ce_cdb_obj_o0 *root_ent_w = ce_cdb_a0->write_begin(db, root_ent);

            ce_cdb_a0->set_str(root_ent_w, ENTITY_NAME, filename);

            {
                uint64_t pos_comp = ce_cdb_a0->create_object(db, POSITION_COMPONENT);
                uint64_t ltw_comp = ce_cdb_a0->create_object(db, LOCAL_TO_WORLD_COMPONENT);
                ce_cdb_a0->objset_add_obj(root_ent_w, ENTITY_COMPONENTS, pos_comp);
                ce_cdb_a0->objset_add_obj(root_ent_w, ENTITY_COMPONENTS, ltw_comp);
            }

            for (int m = 0; m < data->meshes_count; ++m) {
                cgltf_mesh *mesh = &data->meshes[m];

                char mesh_name[128] = {};

                if (mesh->name) {
                    snprintf(mesh_name, CE_ARRAY_LEN(mesh_name), "%s", mesh->name);
                } else {
                    snprintf(mesh_name, CE_ARRAY_LEN(mesh_name), "geom_%d", m);
                }

                for (int p = 0; p < mesh->primitives_count; ++p) {
                    cgltf_primitive *prim = &mesh->primitives[p];

                    char geom_name[128] = {};
                    if (0 == p) {
                        snprintf(geom_name, CE_ARRAY_LEN(geom_name), "%s", mesh_name);
                    } else {
                        snprintf(geom_name, CE_ARRAY_LEN(geom_name), "%s_%d", mesh_name, p);
                    }

                    uint64_t mesh_ent = ce_cdb_a0->create_object(db, ENTITY_TYPE);
                    uint64_t pos_comp = ce_cdb_a0->create_object(db, POSITION_COMPONENT);
                    uint64_t ltw_comp = ce_cdb_a0->create_object(db, LOCAL_TO_WORLD_COMPONENT);
                    uint64_t ltp_comp = ce_cdb_a0->create_object(db, LOCAL_TO_PARENT_COMPONENT);
                    uint64_t static_mesh_comp = ce_cdb_a0->create_object(db, STATIC_MESH_COMPONENT);

                    ce_cdb_obj_o0 *static_mesh_w = ce_cdb_a0->write_begin(db, static_mesh_comp);

                    uint64_t material_obj = ce_hash_lookup(&mat_to_obj,
                                                           (uint64_t) (prim->material),
                                                           0);

                    ce_cdb_a0->set_ref(static_mesh_w, PROP_SCENE_ID, scene_obj);
                    ce_cdb_a0->set_ref(static_mesh_w, PROP_MATERIAL, material_obj);
                    ce_cdb_a0->set_str(static_mesh_w, PROP_MESH, geom_name);
                    ce_cdb_a0->write_commit(static_mesh_w);

                    ce_cdb_obj_o0 *mesh_ent_w = ce_cdb_a0->write_begin(db, mesh_ent);
                    ce_cdb_a0->set_str(mesh_ent_w, ENTITY_NAME, geom_name);
                    ce_cdb_a0->objset_add_obj(mesh_ent_w, ENTITY_COMPONENTS, pos_comp);
                    ce_cdb_a0->objset_add_obj(mesh_ent_w, ENTITY_COMPONENTS, ltw_comp);
                    ce_cdb_a0->objset_add_obj(mesh_ent_w, ENTITY_COMPONENTS, ltp_comp);
                    ce_cdb_a0->objset_add_obj(mesh_ent_w, ENTITY_COMPONENTS, static_mesh_comp);
                    ce_cdb_a0->write_commit(mesh_ent_w);

    }

    cgltf_free(data);
    return true;
}
