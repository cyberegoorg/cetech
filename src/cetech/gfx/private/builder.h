struct render_graph_builder_pass {
    struct ct_render_graph_pass *pass;
    uint8_t viewid;
    uint64_t layer;
    ct_render_frame_buffer_handle_t fb;
};

#define MAX_ATTACHMENTS 8+2
struct render_graph_builder_inst {
    struct render_graph_builder_pass *pass;

    struct ce_hash_t texture_map;

    uint16_t size[2];

    uint8_t attachemnt_used;
    ct_render_texture_handle_t attachemnt[MAX_ATTACHMENTS];
};

static void builder_add_pass(void *inst,
                             struct ct_render_graph_pass *pass,
                             uint64_t layer) {
    struct ct_render_graph_builder *builder = inst;
    struct render_graph_builder_inst *builder_inst = builder->inst;

    uint8_t viewid = _G.viewid++;

    ct_render_frame_buffer_handle_t fb = {.idx = UINT16_MAX};
    const uint8_t n = builder_inst->attachemnt_used;

    if (0 != n) {
        builder_inst->attachemnt_used = 0;

        fb = ct_renderer_a0->create_frame_buffer_from_handles(n,
                                                              builder_inst->attachemnt,
                                                              true);
    }

    ce_array_push(builder_inst->pass,
                  ((struct render_graph_builder_pass) {
                          .pass = pass,
                          .layer = layer,
                          .viewid = viewid,
                          .fb = fb}),
                  _G.alloc);
}

static void builder_execute(void *inst) {
    struct ct_render_graph_builder *builder = inst;
    struct render_graph_builder_inst *builder_inst = builder->inst;

    const uint32_t pass_n = ce_array_size(builder_inst->pass);
    for (int i = 0; i < pass_n; ++i) {
        struct render_graph_builder_pass *pass = &builder_inst->pass[i];

        ct_renderer_a0->touch(pass->viewid);

        if (UINT16_MAX != pass->fb.idx) {
            ct_renderer_a0->set_view_frame_buffer(pass->viewid, pass->fb);
        }

        pass->pass->on_pass(pass->pass,
                            pass->viewid,
                            pass->layer,
                            builder);

    }
}

static void builder_clear(void *inst) {
    struct ct_render_graph_builder *builder = inst;
    struct render_graph_builder_inst *builder_inst = builder->inst;

    const uint32_t pass_n = ce_array_size(builder_inst->pass);
    for (int i = 0; i < pass_n; ++i) {
        struct render_graph_builder_pass *pass = &builder_inst->pass[i];

        if (UINT16_MAX == pass->fb.idx) {
            continue;
        }

        ct_renderer_a0->destroy_frame_buffer(pass->fb);
    }

    ce_array_clean(builder_inst->pass);
    ce_hash_clean(&builder_inst->texture_map);
    builder_inst->attachemnt_used = 0;
}

float ratio_to_coef(ct_render_backbuffer_ratio_t ratio) {
    static float _table[] = {
            [CT_RENDER_BACKBUFFER_RATIO_EQUAL] = 1.0f,
            [CT_RENDER_BACKBUFFER_RATIO_HALF] = 1.0f,
            [CT_RENDER_BACKBUFFER_RATIO_QUARTER] = 1.0f / 2.0f,
            [CT_RENDER_BACKBUFFER_RATIO_EIGHTH] = 1.0f / 4.0f,
            [CT_RENDER_BACKBUFFER_RATIO_SIXTEENTH] = 1.0f / 8.0f,
            [CT_RENDER_BACKBUFFER_RATIO_DOUBLE] = 1.0f / 16.0f,
    };

    return _table[ratio];
}

static void builder_write(void *inst,
                          uint64_t name,
                          struct ct_render_graph_attachment info) {
    struct ct_render_graph_builder *builder = inst;
    struct render_graph_builder_inst *builder_inst = builder->inst;

    const uint32_t samplerFlags = 0
                                  | CT_RENDER_TEXTURE_RT
                                  | CT_RENDER_TEXTURE_MIN_POINT
                                  | CT_RENDER_TEXTURE_MAG_POINT
                                  | CT_RENDER_TEXTURE_MIP_POINT
                                  | CT_RENDER_TEXTURE_U_CLAMP
                                  | CT_RENDER_TEXTURE_V_CLAMP;

    const float coef = ratio_to_coef(info.ratio);

    ct_render_texture_handle_t th;
    th = ct_renderer_a0->create_texture_2d(builder_inst->size[0] * coef,
                                           builder_inst->size[1] * coef,
                                           false, 1, info.format,
                                           samplerFlags, NULL);

    const uint8_t idx = builder_inst->attachemnt_used++;

    builder_inst->attachemnt[idx] = th;

    ce_hash_add(&builder_inst->texture_map, name, th.idx, _G.alloc);
}

static void builder_read(void *inst,
                         uint64_t name) {

}

struct ct_render_texture_handle builder_get_texture(void *inst,
                                                    uint64_t name) {
    struct ct_render_graph_builder *builder = inst;
    struct render_graph_builder_inst *builder_inst = builder->inst;

    return (struct ct_render_texture_handle) {
            .idx = ce_hash_lookup(&builder_inst->texture_map, name, 0)};
}

void builder_set_size(void *inst,
                      uint16_t w,
                      uint16_t h) {
    struct ct_render_graph_builder *builder = inst;
    struct render_graph_builder_inst *builder_inst = builder->inst;

    builder_inst->size[0] = w;
    builder_inst->size[1] = h;

}

void builder_get_size(void *inst,
                      uint16_t *size) {
    struct ct_render_graph_builder *builder = inst;
    struct render_graph_builder_inst *builder_inst = builder->inst;

    memcpy(size, builder_inst->size, sizeof(uint16_t) * 2);
}

struct ct_render_graph_builder_fce render_graph_builder_api = {
        .add_pass = builder_add_pass,
        .execute = builder_execute,
        .clear = builder_clear,
        .create = builder_write,
        .read = builder_read,
        .set_size = builder_set_size,
        .get_size = builder_get_size,
        .get_texture = builder_get_texture,
};

static struct ct_render_graph_builder *create_render_builder() {
    struct ct_render_graph_builder *obj = CE_ALLOC(_G.alloc,
                                                   struct ct_render_graph_builder,
                                                   sizeof(struct ct_render_graph_builder));

    ce_array_push(_G.render_graph_builder_pool,
                  (struct render_graph_builder_inst) {},
                  _G.alloc);
    struct render_graph_builder_inst *inst = &ce_array_back(
            _G.render_graph_builder_pool);

    *obj = (struct ct_render_graph_builder) {
            .call = &render_graph_builder_api,
            .inst = inst
    };

    return obj;
}

static void destroy_render_builder(struct ct_render_graph_builder *builder) {

}