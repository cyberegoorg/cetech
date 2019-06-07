#ifndef CETECH_GFX_H
#define CETECH_GFX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include <bgfx/c99/bgfx.h>

#define CT_GFX_API \
    CE_ID64_0("ct_gfx_a0", 0x945b4a9e424eaecULL)

struct ct_gfx_a0 {
    void (*bgfx_vertex_decl_begin)(bgfx_vertex_decl_t *_decl,
                                   bgfx_renderer_type_t _renderer);

    void (*bgfx_vertex_decl_add)(bgfx_vertex_decl_t *_decl,
                                 bgfx_attrib_t _attrib,
                                 uint8_t _num,
                                 bgfx_attrib_type_t _type,
                                 bool _normalized,
                                 bool _asInt);

    void (*bgfx_vertex_decl_decode)(const bgfx_vertex_decl_t *_decl,
                                    bgfx_attrib_t _attrib,
                                    uint8_t *_num,
                                    bgfx_attrib_type_t *_type,
                                    bool *_normalized,
                                    bool *_asInt);

    bool (*bgfx_vertex_decl_has)(const bgfx_vertex_decl_t *_decl,
                                 bgfx_attrib_t _attrib);

    void (*bgfx_vertex_decl_skip)(bgfx_vertex_decl_t *_decl,
                                  uint8_t _num);

    void (*bgfx_vertex_decl_end)(bgfx_vertex_decl_t *_decl);

    void (*bgfx_vertex_pack)(const float _input[4],
                             bool _inputNormalized,
                             bgfx_attrib_t _attr,
                             const bgfx_vertex_decl_t *_decl,
                             void *_data,
                             uint32_t _index);

    void (*bgfx_vertex_unpack)(float _output[4],
                               bgfx_attrib_t _attr,
                               const bgfx_vertex_decl_t *_decl,
                               const void *_data,
                               uint32_t _index);

    void (*bgfx_vertex_convert)(const bgfx_vertex_decl_t *_destDecl,
                                void *_destData,
                                const bgfx_vertex_decl_t *_srcDecl,
                                const void *_srcData,
                                uint32_t _num);

    uint16_t (*bgfx_weld_vertices)(uint16_t *_output,
                                   const bgfx_vertex_decl_t *_decl,
                                   const void *_data,
                                   uint16_t _num,
                                   float _epsilon);

    uint32_t (*bgfx_topology_convert)(bgfx_topology_convert_t _conversion,
                                      void *_dst,
                                      uint32_t _dstSize,
                                      const void *_indices,
                                      uint32_t _numIndices,
                                      bool _index32);

    void (*bgfx_topology_sort_tri_list)(bgfx_topology_sort_t _sort,
                                        void *_dst,
                                        uint32_t _dstSize,
                                        const float _dir[3],
                                        const float _pos[3],
                                        const void *_vertices,
                                        uint32_t _stride,
                                        const void *_indices,
                                        uint32_t _numIndices,
                                        bool _index32);

    uint8_t (*bgfx_get_supported_renderers)(uint8_t _max,
                                            bgfx_renderer_type_t *_enum);

    const char *(*bgfx_get_renderer_name)(bgfx_renderer_type_t _type);

    void (*bgfx_init_ctor)(bgfx_init_t *_init);

    bool (*bgfx_init)(const bgfx_init_t *_init);

    void (*bgfx_shutdown)(void);

    void (*bgfx_reset)(uint32_t _width,
                       uint32_t _height,
                       uint32_t _flags,
                       bgfx_texture_format_t _format);

    struct bgfx_encoder_s *(*bgfx_begin)(void);

    void (*bgfx_end)(struct bgfx_encoder_s *_encoder);

    uint32_t (*bgfx_frame)(bool _capture);

    bgfx_renderer_type_t (*bgfx_get_renderer_type)(void);

    const bgfx_caps_t *(*bgfx_get_caps)(void);

    const bgfx_stats_t *(*bgfx_get_stats)(void);

    const bgfx_memory_t *(*bgfx_alloc)(uint32_t _size);

    const bgfx_memory_t *(*bgfx_copy)(const void *_data,
                                      uint32_t _size);

    const bgfx_memory_t *(*bgfx_make_ref)(const void *_data,
                                          uint32_t _size);

    const bgfx_memory_t *(*bgfx_make_ref_release)(const void *_data,
                                                  uint32_t _size,
                                                  bgfx_release_fn_t _releaseFn,
                                                  void *_userData);

    void (*bgfx_set_debug)(uint32_t _debug);

    void (*bgfx_dbg_text_clear)(uint8_t _attr,
                                bool _small);

    void (*bgfx_dbg_text_printf)(uint16_t _x,
                                 uint16_t _y,
                                 uint8_t _attr,
                                 const char *_format,
                                 ...);

    void (*bgfx_dbg_text_vprintf)(uint16_t _x,
                                  uint16_t _y,
                                  uint8_t _attr,
                                  const char *_format,
                                  va_list _argList);

    void (*bgfx_dbg_text_image)(uint16_t _x,
                                uint16_t _y,
                                uint16_t _width,
                                uint16_t _height,
                                const void *_data,
                                uint16_t _pitch);

    bgfx_index_buffer_handle_t
    (*bgfx_create_index_buffer)(const bgfx_memory_t *_mem,
                                uint16_t _flags);

    void (*bgfx_set_index_buffer_name)(bgfx_index_buffer_handle_t _handle,
                                       const char *_name,
                                       int32_t _len);

    void (*bgfx_destroy_index_buffer)(bgfx_index_buffer_handle_t _handle);

    bgfx_vertex_buffer_handle_t
    (*bgfx_create_vertex_buffer)(const bgfx_memory_t *_mem,
                                 const bgfx_vertex_decl_t *_decl,
                                 uint16_t _flags);

    void (*bgfx_set_vertex_buffer_name)(bgfx_vertex_buffer_handle_t _handle,
                                        const char *_name,
                                        int32_t _len);

    void (*bgfx_destroy_vertex_buffer)(bgfx_vertex_buffer_handle_t _handle);

    bgfx_dynamic_index_buffer_handle_t
    (*bgfx_create_dynamic_index_buffer)(uint32_t _num,
                                        uint16_t _flags);

    bgfx_dynamic_index_buffer_handle_t
    (*bgfx_create_dynamic_index_buffer_mem)(const bgfx_memory_t *_mem,
                                            uint16_t _flags);

    void
    (*bgfx_update_dynamic_index_buffer)(bgfx_dynamic_index_buffer_handle_t _handle,
                                        uint32_t _startIndex,
                                        const bgfx_memory_t *_mem);

    void
    (*bgfx_destroy_dynamic_index_buffer)(bgfx_dynamic_index_buffer_handle_t _handle);

    bgfx_dynamic_vertex_buffer_handle_t
    (*bgfx_create_dynamic_vertex_buffer)(uint32_t _num,
                                         const bgfx_vertex_decl_t *_decl,
                                         uint16_t _flags);

    bgfx_dynamic_vertex_buffer_handle_t
    (*bgfx_create_dynamic_vertex_buffer_mem)(const bgfx_memory_t *_mem,
                                             const bgfx_vertex_decl_t *_decl,
                                             uint16_t _flags);

    void
    (*bgfx_update_dynamic_vertex_buffer)(bgfx_dynamic_vertex_buffer_handle_t _handle,
                                         uint32_t _startVertex,
                                         const bgfx_memory_t *_mem);

    void
    (*bgfx_destroy_dynamic_vertex_buffer)(bgfx_dynamic_vertex_buffer_handle_t _handle);

    uint32_t (*bgfx_get_avail_transient_index_buffer)(uint32_t _num);

    uint32_t (*bgfx_get_avail_transient_vertex_buffer)(uint32_t _num,
                                                       const bgfx_vertex_decl_t *_decl);

    uint32_t (*bgfx_get_avail_instance_data_buffer)(uint32_t _num,
                                                    uint16_t _stride);

    void
    (*bgfx_alloc_transient_index_buffer)(bgfx_transient_index_buffer_t *_tib,
                                         uint32_t _num);

    void
    (*bgfx_alloc_transient_vertex_buffer)(bgfx_transient_vertex_buffer_t *_tvb,
                                          uint32_t _num,
                                          const bgfx_vertex_decl_t *_decl);

    bool (*bgfx_alloc_transient_buffers)(bgfx_transient_vertex_buffer_t *_tvb,
                                         const bgfx_vertex_decl_t *_decl,
                                         uint32_t _numVertices,
                                         bgfx_transient_index_buffer_t *_tib,
                                         uint32_t _numIndices);

    void (*bgfx_alloc_instance_data_buffer)(bgfx_instance_data_buffer_t *_idb,
                                            uint32_t _num,
                                            uint16_t _stride);

    bgfx_indirect_buffer_handle_t (*bgfx_create_indirect_buffer)(uint32_t _num);

    void (*bgfx_destroy_indirect_buffer)(bgfx_indirect_buffer_handle_t _handle);

    bgfx_shader_handle_t (*bgfx_create_shader)(const bgfx_memory_t *_mem);

    uint16_t (*bgfx_get_shader_uniforms)(bgfx_shader_handle_t _handle,
                                         bgfx_uniform_handle_t *_uniforms,
                                         uint16_t _max);

    void (*bgfx_get_uniform_info)(bgfx_uniform_handle_t _handle,
                                  bgfx_uniform_info_t *_info);

    void (*bgfx_set_shader_name)(bgfx_shader_handle_t _handle,
                                 const char *_name,
                                 int32_t _len);

    void (*bgfx_destroy_shader)(bgfx_shader_handle_t _handle);

    bgfx_program_handle_t (*bgfx_create_program)(bgfx_shader_handle_t _vsh,
                                                 bgfx_shader_handle_t _fsh,
                                                 bool _destroyShaders);

    bgfx_program_handle_t
    (*bgfx_create_compute_program)(bgfx_shader_handle_t _csh,
                                   bool _destroyShaders);

    void (*bgfx_destroy_program)(bgfx_program_handle_t _handle);

    bool (*bgfx_is_texture_valid)(uint16_t _depth,
                                  bool _cubeMap,
                                  uint16_t _numLayers,
                                  bgfx_texture_format_t _format,
                                  uint64_t _flags);

    void (*bgfx_calc_texture_size)(bgfx_texture_info_t *_info,
                                   uint16_t _width,
                                   uint16_t _height,
                                   uint16_t _depth,
                                   bool _cubeMap,
                                   bool _hasMips,
                                   uint16_t _numLayers,
                                   bgfx_texture_format_t _format);

    bgfx_texture_handle_t (*bgfx_create_texture)(const bgfx_memory_t *_mem,
                                                 uint64_t _flags,
                                                 uint8_t _skip,
                                                 bgfx_texture_info_t *_info);

    bgfx_texture_handle_t (*bgfx_create_texture_2d)(uint16_t _width,
                                                    uint16_t _height,
                                                    bool _hasMips,
                                                    uint16_t _numLayers,
                                                    bgfx_texture_format_t _format,
                                                    uint64_t _flags,
                                                    const bgfx_memory_t *_mem);

    bgfx_texture_handle_t
    (*bgfx_create_texture_2d_scaled)(bgfx_backbuffer_ratio_t _ratio,
                                     bool _hasMips,
                                     uint16_t _numLayers,
                                     bgfx_texture_format_t _format,
                                     uint64_t _flags);

    bgfx_texture_handle_t (*bgfx_create_texture_3d)(uint16_t _width,
                                                    uint16_t _height,
                                                    uint16_t _depth,
                                                    bool _hasMips,
                                                    bgfx_texture_format_t _format,
                                                    uint64_t _flags,
                                                    const bgfx_memory_t *_mem);

    bgfx_texture_handle_t (*bgfx_create_texture_cube)(uint16_t _size,
                                                      bool _hasMips,
                                                      uint16_t _numLayers,
                                                      bgfx_texture_format_t _format,
                                                      uint64_t _flags,
                                                      const bgfx_memory_t *_mem);

    void (*bgfx_update_texture_2d)(bgfx_texture_handle_t _handle,
                                   uint16_t _layer,
                                   uint8_t _mip,
                                   uint16_t _x,
                                   uint16_t _y,
                                   uint16_t _width,
                                   uint16_t _height,
                                   const bgfx_memory_t *_mem,
                                   uint16_t _pitch);

    void (*bgfx_update_texture_3d)(bgfx_texture_handle_t _handle,
                                   uint8_t _mip,
                                   uint16_t _x,
                                   uint16_t _y,
                                   uint16_t _z,
                                   uint16_t _width,
                                   uint16_t _height,
                                   uint16_t _depth,
                                   const bgfx_memory_t *_mem);

    void (*bgfx_update_texture_cube)(bgfx_texture_handle_t _handle,
                                     uint16_t _layer,
                                     uint8_t _side,
                                     uint8_t _mip,
                                     uint16_t _x,
                                     uint16_t _y,
                                     uint16_t _width,
                                     uint16_t _height,
                                     const bgfx_memory_t *_mem,
                                     uint16_t _pitch);

    uint32_t (*bgfx_read_texture)(bgfx_texture_handle_t _handle,
                                  void *_data,
                                  uint8_t _mip);

    void (*bgfx_set_texture_name)(bgfx_texture_handle_t _handle,
                                  const char *_name,
                                  int32_t _len);

    void (*bgfx_destroy_texture)(bgfx_texture_handle_t _handle);

    bgfx_frame_buffer_handle_t (*bgfx_create_frame_buffer)(uint16_t _width,
                                                           uint16_t _height,
                                                           bgfx_texture_format_t _format,
                                                           uint64_t _textureFlags);

    void (*bgfx_set_frame_buffer_name)(bgfx_frame_buffer_handle_t _handle,
                                       const char *_name,
                                       int32_t _len);

    bgfx_frame_buffer_handle_t
    (*bgfx_create_frame_buffer_scaled)(bgfx_backbuffer_ratio_t _ratio,
                                       bgfx_texture_format_t _format,
                                       uint64_t _textureFlags);

    bgfx_frame_buffer_handle_t
    (*bgfx_create_frame_buffer_from_handles)(uint8_t _num,
                                             const bgfx_texture_handle_t *_handles,
                                             bool _destroyTextures);

    bgfx_frame_buffer_handle_t
    (*bgfx_create_frame_buffer_from_attachment)(uint8_t _num,
                                                const bgfx_attachment_t *_attachment,
                                                bool _destroyTextures);

    bgfx_frame_buffer_handle_t (*bgfx_create_frame_buffer_from_nwh)(void *_nwh,
                                                                    uint16_t _width,
                                                                    uint16_t _height,
                                                                    bgfx_texture_format_t _format,
                                                                    bgfx_texture_format_t _depthFormat);

    bgfx_texture_handle_t
    (*bgfx_get_texture)(bgfx_frame_buffer_handle_t _handle,
                        uint8_t _attachment);

    void (*bgfx_destroy_frame_buffer)(bgfx_frame_buffer_handle_t _handle);

    bgfx_uniform_handle_t (*bgfx_create_uniform)(const char *_name,
                                                 bgfx_uniform_type_t _type,
                                                 uint16_t _num);

    void (*bgfx_destroy_uniform)(bgfx_uniform_handle_t _handle);

    bgfx_occlusion_query_handle_t (*bgfx_create_occlusion_query)(void);

    bgfx_occlusion_query_result_t
    (*bgfx_get_result)(bgfx_occlusion_query_handle_t _handle,
                       int32_t *_result);

    void (*bgfx_destroy_occlusion_query)(bgfx_occlusion_query_handle_t _handle);

    void (*bgfx_set_palette_color)(uint8_t _index,
                                   const float _rgba[4]);

    void (*bgfx_set_view_name)(bgfx_view_id_t _id,
                               const char *_name);

    void (*bgfx_set_view_rect)(bgfx_view_id_t _id,
                               uint16_t _x,
                               uint16_t _y,
                               uint16_t _width,
                               uint16_t _height);

    void (*bgfx_set_view_rect_auto)(bgfx_view_id_t _id,
                                    uint16_t _x,
                                    uint16_t _y,
                                    bgfx_backbuffer_ratio_t _ratio);

    void (*bgfx_set_view_scissor)(bgfx_view_id_t _id,
                                  uint16_t _x,
                                  uint16_t _y,
                                  uint16_t _width,
                                  uint16_t _height);

    void (*bgfx_set_view_clear)(bgfx_view_id_t _id,
                                uint16_t _flags,
                                uint32_t _rgba,
                                float _depth,
                                uint8_t _stencil);

    void (*bgfx_set_view_clear_mrt)(bgfx_view_id_t _id,
                                    uint16_t _flags,
                                    float _depth,
                                    uint8_t _stencil,
                                    uint8_t _0,
                                    uint8_t _1,
                                    uint8_t _2,
                                    uint8_t _3,
                                    uint8_t _4,
                                    uint8_t _5,
                                    uint8_t _6,
                                    uint8_t _7);

    void (*bgfx_set_view_mode)(bgfx_view_id_t _id,
                               bgfx_view_mode_t _mode);

    void (*bgfx_set_view_frame_buffer)(bgfx_view_id_t _id,
                                       bgfx_frame_buffer_handle_t _handle);

    void (*bgfx_set_view_transform)(bgfx_view_id_t _id,
                                    const void *_view,
                                    const void *_proj);

    void (*bgfx_set_view_order)(bgfx_view_id_t _id,
                                uint16_t _num,
                                const bgfx_view_id_t *_order);

    void (*bgfx_reset_view)(bgfx_view_id_t _id);

    void (*bgfx_set_marker)(const char *_marker);

    void (*bgfx_set_state)(uint64_t _state,
                           uint32_t _rgba);

    void (*bgfx_set_condition)(bgfx_occlusion_query_handle_t _handle,
                               bool _visible);

    void (*bgfx_set_stencil)(uint32_t _fstencil,
                             uint32_t _bstencil);

    uint16_t (*bgfx_set_scissor)(uint16_t _x,
                                 uint16_t _y,
                                 uint16_t _width,
                                 uint16_t _height);

    void (*bgfx_set_scissor_cached)(uint16_t _cache);

    uint32_t (*bgfx_set_transform)(const void *_mtx,
                                   uint16_t _num);

    uint32_t (*bgfx_alloc_transform)(bgfx_transform_t *_transform,
                                     uint16_t _num);

    void (*bgfx_set_transform_cached)(uint32_t _cache,
                                      uint16_t _num);

    void (*bgfx_set_uniform)(bgfx_uniform_handle_t _handle,
                             const void *_value,
                             uint16_t _num);

    void (*bgfx_set_index_buffer)(bgfx_index_buffer_handle_t _handle,
                                  uint32_t _firstIndex,
                                  uint32_t _numIndices);

    void
    (*bgfx_set_dynamic_index_buffer)(bgfx_dynamic_index_buffer_handle_t _handle,
                                     uint32_t _firstIndex,
                                     uint32_t _numIndices);

    void
    (*bgfx_set_transient_index_buffer)(const bgfx_transient_index_buffer_t *_tib,
                                       uint32_t _firstIndex,
                                       uint32_t _numIndices);

    void (*bgfx_set_vertex_buffer)(uint8_t _stream,
                                   bgfx_vertex_buffer_handle_t _handle,
                                   uint32_t _startVertex,
                                   uint32_t _numVertices);

    void (*bgfx_set_dynamic_vertex_buffer)(uint8_t _stream,
                                           bgfx_dynamic_vertex_buffer_handle_t _handle,
                                           uint32_t _startVertex,
                                           uint32_t _numVertices);

    void (*bgfx_set_transient_vertex_buffer)(uint8_t _stream,
                                             const bgfx_transient_vertex_buffer_t *_tvb,
                                             uint32_t _startVertex,
                                             uint32_t _numVertices);

    void (*bgfx_set_vertex_count)(uint32_t _numVertices);

    void
    (*bgfx_set_instance_data_buffer)(const bgfx_instance_data_buffer_t *_idb,
                                     uint32_t _start,
                                     uint32_t _num);

    void
    (*bgfx_set_instance_data_from_vertex_buffer)(bgfx_vertex_buffer_handle_t _handle,
                                                 uint32_t _startVertex,
                                                 uint32_t _num);

    void
    (*bgfx_set_instance_data_from_dynamic_vertex_buffer)(bgfx_dynamic_vertex_buffer_handle_t _handle,
                                                         uint32_t _startVertex,
                                                         uint32_t _num);

    void (*bgfx_set_instance_count)(uint32_t _numInstances);

    void (*bgfx_set_texture)(uint8_t _stage,
                             bgfx_uniform_handle_t _sampler,
                             bgfx_texture_handle_t _handle,
                             uint32_t _flags);

    void (*bgfx_touch)(bgfx_view_id_t _id);

    void (*bgfx_submit)(bgfx_view_id_t _id,
                        bgfx_program_handle_t _handle,
                        uint32_t _depth,
                        bool _preserveState);

    void (*bgfx_submit_occlusion_query)(bgfx_view_id_t _id,
                                        bgfx_program_handle_t _program,
                                        bgfx_occlusion_query_handle_t _occlusionQuery,
                                        uint32_t _depth,
                                        bool _preserveState);

    void (*bgfx_submit_indirect)(bgfx_view_id_t _id,
                                 bgfx_program_handle_t _handle,
                                 bgfx_indirect_buffer_handle_t _indirectHandle,
                                 uint16_t _start,
                                 uint16_t _num,
                                 uint32_t _depth,
                                 bool _preserveState);

    void (*bgfx_set_image)(uint8_t _stage,
                           bgfx_texture_handle_t _handle,
                           uint8_t _mip,
                           bgfx_access_t _access,
                           bgfx_texture_format_t _format);

    void (*bgfx_set_compute_index_buffer)(uint8_t _stage,
                                          bgfx_index_buffer_handle_t _handle,
                                          bgfx_access_t _access);

    void (*bgfx_set_compute_vertex_buffer)(uint8_t _stage,
                                           bgfx_vertex_buffer_handle_t _handle,
                                           bgfx_access_t _access);

    void (*bgfx_set_compute_dynamic_index_buffer)(uint8_t _stage,
                                                  bgfx_dynamic_index_buffer_handle_t _handle,
                                                  bgfx_access_t _access);

    void (*bgfx_set_compute_dynamic_vertex_buffer)(uint8_t _stage,
                                                   bgfx_dynamic_vertex_buffer_handle_t _handle,
                                                   bgfx_access_t _access);

    void (*bgfx_set_compute_indirect_buffer)(uint8_t _stage,
                                             bgfx_indirect_buffer_handle_t _handle,
                                             bgfx_access_t _access);

    void (*bgfx_dispatch)(bgfx_view_id_t _id,
                          bgfx_program_handle_t _handle,
                          uint32_t _numX,
                          uint32_t _numY,
                          uint32_t _numZ);

    void (*bgfx_dispatch_indirect)(bgfx_view_id_t _id,
                                   bgfx_program_handle_t _handle,
                                   bgfx_indirect_buffer_handle_t _indirectHandle,
                                   uint16_t _start,
                                   uint16_t _num);

    void (*bgfx_discard)(void);

    void (*bgfx_blit)(bgfx_view_id_t _id,
                      bgfx_texture_handle_t _dst,
                      uint8_t _dstMip,
                      uint16_t _dstX,
                      uint16_t _dstY,
                      uint16_t _dstZ,
                      bgfx_texture_handle_t _src,
                      uint8_t _srcMip,
                      uint16_t _srcX,
                      uint16_t _srcY,
                      uint16_t _srcZ,
                      uint16_t _width,
                      uint16_t _height,
                      uint16_t _depth);

    void (*bgfx_encoder_set_marker)(struct bgfx_encoder_s *_encoder,
                                    const char *_marker);

    void (*bgfx_encoder_set_state)(struct bgfx_encoder_s *_encoder,
                                   uint64_t _state,
                                   uint32_t _rgba);

    void (*bgfx_encoder_set_condition)(struct bgfx_encoder_s *_encoder,
                                       bgfx_occlusion_query_handle_t _handle,
                                       bool _visible);

    void (*bgfx_encoder_set_stencil)(struct bgfx_encoder_s *_encoder,
                                     uint32_t _fstencil,
                                     uint32_t _bstencil);

    uint16_t (*bgfx_encoder_set_scissor)(struct bgfx_encoder_s *_encoder,
                                         uint16_t _x,
                                         uint16_t _y,
                                         uint16_t _width,
                                         uint16_t _height);

    void (*bgfx_encoder_set_scissor_cached)(struct bgfx_encoder_s *_encoder,
                                            uint16_t _cache);

    uint32_t (*bgfx_encoder_set_transform)(struct bgfx_encoder_s *_encoder,
                                           const void *_mtx,
                                           uint16_t _num);

    uint32_t (*bgfx_encoder_alloc_transform)(struct bgfx_encoder_s *_encoder,
                                             bgfx_transform_t *_transform,
                                             uint16_t _num);

    void (*bgfx_encoder_set_transform_cached)(struct bgfx_encoder_s *_encoder,
                                              uint32_t _cache,
                                              uint16_t _num);

    void (*bgfx_encoder_set_uniform)(struct bgfx_encoder_s *_encoder,
                                     bgfx_uniform_handle_t _handle,
                                     const void *_value,
                                     uint16_t _num);

    void (*bgfx_encoder_set_index_buffer)(struct bgfx_encoder_s *_encoder,
                                          bgfx_index_buffer_handle_t _handle,
                                          uint32_t _firstIndex,
                                          uint32_t _numIndices);

    void
    (*bgfx_encoder_set_dynamic_index_buffer)(struct bgfx_encoder_s *_encoder,
                                             bgfx_dynamic_index_buffer_handle_t _handle,
                                             uint32_t _firstIndex,
                                             uint32_t _numIndices);

    void
    (*bgfx_encoder_set_transient_index_buffer)(struct bgfx_encoder_s *_encoder,
                                               const bgfx_transient_index_buffer_t *_tib,
                                               uint32_t _firstIndex,
                                               uint32_t _numIndices);

    void (*bgfx_encoder_set_vertex_buffer)(struct bgfx_encoder_s *_encoder,
                                           uint8_t _stream,
                                           bgfx_vertex_buffer_handle_t _handle,
                                           uint32_t _startVertex,
                                           uint32_t _numVertices);

    void
    (*bgfx_encoder_set_dynamic_vertex_buffer)(struct bgfx_encoder_s *_encoder,
                                              uint8_t _stream,
                                              bgfx_dynamic_vertex_buffer_handle_t _handle,
                                              uint32_t _startVertex,
                                              uint32_t _numVertices);

    void
    (*bgfx_encoder_set_transient_vertex_buffer)(struct bgfx_encoder_s *_encoder,
                                                uint8_t _stream,
                                                const bgfx_transient_vertex_buffer_t *_tvb,
                                                uint32_t _startVertex,
                                                uint32_t _numVertices);

    void (*bgfx_encoder_set_vertex_count)(struct bgfx_encoder_s *_encoder,
                                          uint32_t _numVertices);

    void
    (*bgfx_encoder_set_instance_data_buffer)(struct bgfx_encoder_s *_encoder,
                                             const bgfx_instance_data_buffer_t *_idb,
                                             uint32_t _start,
                                             uint32_t _num);

    void
    (*bgfx_encoder_set_instance_data_from_vertex_buffer)(struct bgfx_encoder_s *_encoder,
                                                         bgfx_vertex_buffer_handle_t _handle,
                                                         uint32_t _startVertex,
                                                         uint32_t _num);

    void
    (*bgfx_encoder_set_instance_data_from_dynamic_vertex_buffer)(struct bgfx_encoder_s *_encoder,
                                                                 bgfx_dynamic_vertex_buffer_handle_t _handle,
                                                                 uint32_t _startVertex,
                                                                 uint32_t _num);

    void (*bgfx_encoder_set_texture)(struct bgfx_encoder_s *_encoder,
                                     uint8_t _stage,
                                     bgfx_uniform_handle_t _sampler,
                                     bgfx_texture_handle_t _handle,
                                     uint32_t _flags);

    void (*bgfx_encoder_touch)(struct bgfx_encoder_s *_encoder,
                               bgfx_view_id_t _id);

    void (*bgfx_encoder_submit)(struct bgfx_encoder_s *_encoder,
                                bgfx_view_id_t _id,
                                bgfx_program_handle_t _handle,
                                uint32_t _depth,
                                bool _preserveState);

    void (*bgfx_encoder_submit_occlusion_query)(struct bgfx_encoder_s *_encoder,
                                                bgfx_view_id_t _id,
                                                bgfx_program_handle_t _program,
                                                bgfx_occlusion_query_handle_t _occlusionQuery,
                                                uint32_t _depth,
                                                bool _preserveState);

    void (*bgfx_encoder_submit_indirect)(struct bgfx_encoder_s *_encoder,
                                         bgfx_view_id_t _id,
                                         bgfx_program_handle_t _handle,
                                         bgfx_indirect_buffer_handle_t _indirectHandle,
                                         uint16_t _start,
                                         uint16_t _num,
                                         uint32_t _depth,
                                         bool _preserveState);

    void (*bgfx_encoder_set_image)(struct bgfx_encoder_s *_encoder,
                                   uint8_t _stage,
                                   bgfx_texture_handle_t _handle,
                                   uint8_t _mip,
                                   bgfx_access_t _access,
                                   bgfx_texture_format_t _format);

    void
    (*bgfx_encoder_set_compute_index_buffer)(struct bgfx_encoder_s *_encoder,
                                             uint8_t _stage,
                                             bgfx_index_buffer_handle_t _handle,
                                             bgfx_access_t _access);

    void
    (*bgfx_encoder_set_compute_vertex_buffer)(struct bgfx_encoder_s *_encoder,
                                              uint8_t _stage,
                                              bgfx_vertex_buffer_handle_t _handle,
                                              bgfx_access_t _access);

    void
    (*bgfx_encoder_set_compute_dynamic_index_buffer)(struct bgfx_encoder_s *_encoder,
                                                     uint8_t _stage,
                                                     bgfx_dynamic_index_buffer_handle_t _handle,
                                                     bgfx_access_t _access);

    void
    (*bgfx_encoder_set_compute_dynamic_vertex_buffer)(struct bgfx_encoder_s *_encoder,
                                                      uint8_t _stage,
                                                      bgfx_dynamic_vertex_buffer_handle_t _handle,
                                                      bgfx_access_t _access);

    void
    (*bgfx_encoder_set_compute_indirect_buffer)(struct bgfx_encoder_s *_encoder,
                                                uint8_t _stage,
                                                bgfx_indirect_buffer_handle_t _handle,
                                                bgfx_access_t _access);

    void (*bgfx_encoder_dispatch)(struct bgfx_encoder_s *_encoder,
                                  bgfx_view_id_t _id,
                                  bgfx_program_handle_t _handle,
                                  uint32_t _numX,
                                  uint32_t _numY,
                                  uint32_t _numZ);

    void (*bgfx_encoder_dispatch_indirect)(struct bgfx_encoder_s *_encoder,
                                           bgfx_view_id_t _id,
                                           bgfx_program_handle_t _handle,
                                           bgfx_indirect_buffer_handle_t _indirectHandle,
                                           uint16_t _start,
                                           uint16_t _num);

    void (*bgfx_encoder_discard)(struct bgfx_encoder_s *_encoder);

    void (*bgfx_encoder_blit)(struct bgfx_encoder_s *_encoder,
                              bgfx_view_id_t _id,
                              bgfx_texture_handle_t _dst,
                              uint8_t _dstMip,
                              uint16_t _dstX,
                              uint16_t _dstY,
                              uint16_t _dstZ,
                              bgfx_texture_handle_t _src,
                              uint8_t _srcMip,
                              uint16_t _srcX,
                              uint16_t _srcY,
                              uint16_t _srcZ,
                              uint16_t _width,
                              uint16_t _height,
                              uint16_t _depth);

    void (*bgfx_request_screen_shot)(bgfx_frame_buffer_handle_t _handle,
                                     const char *_filePath);
};

CE_MODULE(ct_gfx_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_GFX_H

