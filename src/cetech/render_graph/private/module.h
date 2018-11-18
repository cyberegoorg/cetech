struct render_graph_module_inst {
    uint8_t *pass;
};

static void add_pass(void *inst,
                     void *pass,
                     uint64_t size) {
    struct ct_render_graph_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;

    struct ct_render_graph_pass *p = pass;
    p->size = size;

    ce_array_push_n(module_inst->pass, pass, size, _G.alloc);
}

static void module_on_setup(void *inst,
                            struct ct_render_graph_builder *builder) {
    struct ct_render_graph_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;

    const uint32_t pass_n = ce_array_size(module_inst->pass);
    for (int i = 0; i < pass_n;) {
        struct ct_render_graph_pass *pass = (struct ct_render_graph_pass *) &module_inst->pass[i];
        pass->on_setup(pass, builder);

        i += pass->size;
    }
}

struct ct_render_graph_module_fce render_graph_module_api = {
        .add_pass= add_pass,
        .on_setup = module_on_setup,
};

static struct ct_render_graph_module *create_module() {
    struct ct_render_graph_module *obj = CE_ALLOC(_G.alloc,
                                                  struct ct_render_graph_module,
                                                  sizeof(struct ct_render_graph_module));

    ce_array_push(_G.render_graph_module_pool,
                  (struct render_graph_module_inst) {}, _G.alloc);

    struct render_graph_module_inst *inst = &ce_array_back(
            _G.render_graph_module_pool);

    *obj = (struct ct_render_graph_module) {
            .call = &render_graph_module_api,
            .inst = inst,
    };

    return obj;
}

static void destroy_module(struct ct_render_graph_module *module) {

}