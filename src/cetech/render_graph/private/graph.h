typedef struct render_graph_o {
    ct_rg_module_t0 *module;
} render_graph_o;

void set_module(void *inst,
                struct ct_rg_module_t0 *module) {
    ct_rg_t0 *rg = inst;
    render_graph_o *rg_inst = rg->inst;

    rg_inst->module = module;
}

void graph_setup(void *inst,
                 struct ct_rg_builder_t0 *builder) {
    ct_rg_t0 *rg = inst;
    render_graph_o *rg_inst = rg->inst;

    rg_inst->module->on_setup(rg_inst->module, builder);
}

static struct ct_rg_t0 *create_render_graph() {
    ct_rg_t0 *obj = CE_ALLOC(_G.alloc, ct_rg_t0, sizeof(ct_rg_t0));

    render_graph_o *inst = CE_ALLOC(_G.alloc,
                                           render_graph_o,
                                           sizeof(render_graph_o));

    *inst = (render_graph_o) {};

    *obj = (ct_rg_t0) {
            .inst = inst,
            .set_module = set_module,
            .setup = graph_setup,
    };

    return obj;
}

static void destroy_render_graph(ct_rg_t0 *render_graph) {
    ct_rg_t0 *rg = render_graph;
    render_graph_o *rg_inst = rg->inst;

    CE_FREE(_G.alloc, rg_inst);
    CE_FREE(_G.alloc, rg);
}
