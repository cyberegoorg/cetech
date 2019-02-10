typedef struct render_graph_inst {
    struct ct_rg_module *module;
}render_graph_inst;

void set_module(void *inst,
                struct ct_rg_module *module) {
    struct ct_rg *rg = inst;
    struct render_graph_inst *rg_inst = rg->inst;

    rg_inst->module = module;
}

void graph_setup(void *inst,
                 struct ct_rg_builder_t0 *builder) {
    struct ct_rg *rg = inst;
    struct render_graph_inst *rg_inst = rg->inst;

    rg_inst->module->on_setup(rg_inst->module, builder);
}

static struct ct_rg *create_render_graph() {
    struct ct_rg *obj = CE_ALLOC(_G.alloc,
                                           struct ct_rg,
                                           sizeof(ct_rg));

    struct render_graph_inst *inst = CE_ALLOC(_G.alloc,
                                              struct render_graph_inst,
                                              sizeof(render_graph_inst));

    *inst = (render_graph_inst){};

    *obj = (ct_rg) {
            .inst = inst,
            .set_module = set_module,
            .setup = graph_setup,
    };

    return obj;
}

static void destroy_render_graph(ct_rg *render_graph) {
    struct ct_rg *rg = render_graph;
    struct render_graph_inst *rg_inst = rg->inst;

    CE_FREE(_G.alloc, rg_inst);
    CE_FREE(_G.alloc, rg);
}
