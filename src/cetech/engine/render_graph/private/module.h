struct render_graph_module_inst {
    struct ct_render_graph_pass **pass;
};

static void add_pass(void *inst,
                     struct ct_render_graph_pass *pass) {
    struct ct_render_graph_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;

    ct_array_push(module_inst->pass, pass, _G.alloc);
}

static void module_on_setup(void *inst,
                            struct ct_render_graph_builder *builder) {
    struct ct_render_graph_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;
    const uint32_t pass_n = ct_array_size(module_inst->pass);
    for (int i = 0; i < pass_n; ++i) {
        struct ct_render_graph_pass *pass = module_inst->pass[i];
        pass->call->on_setup(pass, builder);
    }
}

struct ct_render_graph_module_fce render_graph_module_api = {
        .add_pass= add_pass,
        .on_setup = module_on_setup,
};

static struct ct_render_graph_module *create_module() {
    struct ct_render_graph_module *obj = CT_ALLOC(_G.alloc,
                                                  struct ct_render_graph_module,
                                                  sizeof(struct ct_render_graph_module));

    ct_array_push(_G.render_graph_module_pool,
                  (struct render_graph_module_inst) {0}, _G.alloc);

    struct render_graph_module_inst *inst = &ct_array_back(
            _G.render_graph_module_pool);

    *obj = (struct ct_render_graph_module) {
            .call = &render_graph_module_api,
            .inst = inst,
    };

    return obj;
}

static void destroy_module(struct ct_render_graph_module *module) {

}