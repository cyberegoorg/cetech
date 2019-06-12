#include <cetech/debugdraw/dd.h>

typedef struct render_graph_builder_pass {
    ct_rg_pass_t0 *pass;
    uint8_t viewid;
    uint64_t layer;
    bgfx_frame_buffer_handle_t fb;
} render_graph_builder_pass;

#define MAX_ATTACHMENTS 8+2
typedef struct render_graph_builder_inst {
    render_graph_builder_pass *pass;

    ce_hash_t texture_map;

    ce_hash_t layer_map;

    uint16_t size[2];

    uint8_t attachemnt_used;
    bgfx_texture_handle_t attachemnt[MAX_ATTACHMENTS];
} render_graph_builder_inst;

#define _DEFAULT \
     CE_ID64_0("default", 0xf27605035974b5ecULL)

static void builder_add_pass(void *inst,
                             struct ct_rg_pass_t0 *pass,
                             uint64_t layer) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    uint8_t viewid = 0;

    if (!layer) {
        viewid = ct_renderer_a0->new_viewid();
    } else {
        viewid = ce_hash_lookup(&builder_inst->layer_map, layer, UINT8_MAX);
        if (UINT8_MAX == viewid) {
            viewid = ct_renderer_a0->new_viewid();
            ce_hash_add(&builder_inst->layer_map, layer, viewid, _G.alloc);
        }
    }

    bgfx_frame_buffer_handle_t fb = {.idx = UINT16_MAX};
    if (pass->on_pass) {
        const uint8_t n = builder_inst->attachemnt_used;
        if (0 != n) {
            fb = ct_gfx_a0->bgfx_create_frame_buffer_from_handles(n,
                                                                  builder_inst->attachemnt,
                                                                  true);
        }
    }

    ce_array_push(builder_inst->pass,
                  ((render_graph_builder_pass) {
                          .pass = pass,
                          .layer = layer,
                          .viewid = viewid,
                          .fb = fb}),
                  _G.alloc);

    builder_inst->attachemnt_used = 0;
}

static void builder_execute(void *inst,
                            ct_camera_data_t0 *main_camera) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    const uint32_t pass_n = ce_array_size(builder_inst->pass);

    for (int i = 0; i < pass_n; ++i) {
        struct render_graph_builder_pass *pass = &builder_inst->pass[i];

        if (UINT16_MAX == pass->fb.idx) {
            continue;
        }

        ct_gfx_a0->bgfx_set_view_frame_buffer(pass->viewid, pass->fb);
        ct_gfx_a0->bgfx_touch(pass->viewid);

        pass->pass->on_pass(pass->pass, pass->viewid, pass->layer, main_camera, builder);
    }

    for (int i = 0; i < pass_n; ++i) {
        struct render_graph_builder_pass *pass = &builder_inst->pass[i];

        if (UINT16_MAX == pass->fb.idx) {
            continue;
        }

        ct_gfx_a0->bgfx_set_view_frame_buffer(pass->viewid, pass->fb);
        ct_gfx_a0->bgfx_touch(pass->viewid);

        pass->pass->on_pass(pass->pass, pass->viewid, pass->layer, main_camera, builder);
    }
}

static void builder_clear(void *inst) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    const uint32_t pass_n = ce_array_size(builder_inst->pass);
    for (int i = 0; i < pass_n; ++i) {
        struct render_graph_builder_pass *pass = &builder_inst->pass[i];

        if (UINT16_MAX == pass->fb.idx) {
            continue;
        }

        ct_gfx_a0->bgfx_destroy_frame_buffer(pass->fb);
    }

    ce_array_clean(builder_inst->pass);
    ce_hash_clean(&builder_inst->texture_map);
    ce_hash_clean(&builder_inst->layer_map);
    builder_inst->attachemnt_used = 0;
}

float ratio_to_coef(bgfx_backbuffer_ratio_t ratio) {
    static float _table[] = {
            [BGFX_BACKBUFFER_RATIO_EQUAL] = 1.0f,
            [BGFX_BACKBUFFER_RATIO_HALF] = 1.0f,
            [BGFX_BACKBUFFER_RATIO_QUARTER] = 1.0f / 2.0f,
            [BGFX_BACKBUFFER_RATIO_EIGHTH] = 1.0f / 4.0f,
            [BGFX_BACKBUFFER_RATIO_SIXTEENTH] = 1.0f / 8.0f,
            [BGFX_BACKBUFFER_RATIO_DOUBLE] = 1.0f / 16.0f,
    };

    return _table[ratio];
}

static void builder_create(void *inst,
                           uint64_t name,
                           struct ct_rg_attachment_t0 info) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    const uint64_t samplerFlags = 0
                                  | BGFX_TEXTURE_RT
                                  | BGFX_SAMPLER_MIN_POINT
                                  | BGFX_SAMPLER_MAG_POINT
                                  | BGFX_SAMPLER_MIP_POINT
                                  | BGFX_SAMPLER_U_CLAMP
                                  | BGFX_SAMPLER_V_CLAMP;

    const float coef = ratio_to_coef(info.ratio);

    bgfx_texture_handle_t th;
    th = ct_gfx_a0->bgfx_create_texture_2d(builder_inst->size[0] * coef,
                                           builder_inst->size[1] * coef,
                                           false, 1, info.format,
                                           samplerFlags, NULL);

    const uint8_t idx = builder_inst->attachemnt_used++;
    builder_inst->attachemnt[idx] = th;

    ce_hash_add(&builder_inst->texture_map, name, th.idx, _G.alloc);
}

static void builder_write(void *inst,
                          uint64_t name) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    uint32_t tidx = ce_hash_lookup(&builder_inst->texture_map, name, 0);
    const uint8_t idx = builder_inst->attachemnt_used++;
    builder_inst->attachemnt[idx] = (bgfx_texture_handle_t) {.idx=tidx};
}

static void builder_read(void *inst,
                         uint64_t name) {
}

bgfx_texture_handle_t builder_get_texture(void *inst,
                                          uint64_t name) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    return (bgfx_texture_handle_t) {
            .idx = ce_hash_lookup(&builder_inst->texture_map, name, 0)};
}

void builder_set_size(void *inst,
                      uint16_t w,
                      uint16_t h) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    builder_inst->size[0] = w;
    builder_inst->size[1] = h;
}

void builder_get_size(void *inst,
                      uint16_t *size) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    memcpy(size, builder_inst->size, sizeof(uint16_t) * 2);
}

static uint8_t get_layer_viewid(void *inst,
                                uint64_t layer_name) {
    ct_rg_builder_t0 *builder = inst;
    render_graph_builder_inst *builder_inst = builder->inst;

    return ce_hash_lookup(&builder_inst->layer_map, layer_name, 0);
}

static struct ct_rg_builder_t0 *create_render_builder() {
    ct_rg_builder_t0 *obj = CE_ALLOC(_G.alloc,
                                     ct_rg_builder_t0,
                                     sizeof(ct_rg_builder_t0));

    render_graph_builder_inst *inst = CE_ALLOC(_G.alloc,
                                               struct render_graph_builder_inst,
                                               sizeof(render_graph_builder_inst));


    *inst = (render_graph_builder_inst) {};

    *obj = (ct_rg_builder_t0) {
            .add_pass = builder_add_pass,
            .execute = builder_execute,
            .clear = builder_clear,
            .create = builder_create,
            .write = builder_write,
            .read = builder_read,
            .set_size = builder_set_size,
            .get_size = builder_get_size,
            .get_layer_viewid = get_layer_viewid,
            .get_texture = builder_get_texture,
            .inst = inst
    };

    return obj;
}

static void destroy_render_builder(ct_rg_builder_t0 *builder) {

}