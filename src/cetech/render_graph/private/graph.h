struct render_graph_inst {
    struct ct_render_graph_module **modules;
};

void add_module(void *inst,
                struct ct_render_graph_module *module) {
    struct ct_render_graph *rg = inst;
    struct render_graph_inst *rg_inst = rg->inst;

    ce_array_push(rg_inst->modules, module, _G.alloc);
}

void graph_setup(void *inst,
                 struct ct_render_graph_builder *builder) {
    struct ct_render_graph *rg = inst;
    struct render_graph_inst *rg_inst = rg->inst;

    const uint32_t modules_n = ce_array_size(rg_inst->modules);
    for (int i = 0; i < modules_n; ++i) {
        struct ct_render_graph_module *module = rg_inst->modules[i];
        module->call->on_setup(module, builder);
    }
}

struct ct_render_graph_fce render_graph_fce = {
        .add_module = add_module,
        .setup = graph_setup,
};


static struct ct_render_graph *create_render_graph() {
    struct ct_render_graph *obj = CE_ALLOC(_G.alloc,
                                           struct ct_render_graph,
                                           sizeof(struct ct_render_graph));

    ce_array_push(_G.render_graph_pool, (struct render_graph_inst) {},
                  _G.alloc);
    struct render_graph_inst *inst = &ce_array_back(_G.render_graph_pool);

    *obj = (struct ct_render_graph) {
            .call = &render_graph_fce,
            .inst = inst
    };

    return obj;
}

static void destroy_render_graph(struct ct_render_graph *render_graph) {

}
