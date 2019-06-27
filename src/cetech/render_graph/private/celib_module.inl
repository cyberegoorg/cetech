
static void _free_render_graph_module_inst(render_graph_module_inst *inst,
                                           const ce_alloc_t0 *alloc) {
    ce_array_free(inst->pass, alloc);
    ce_array_free(inst->modules, alloc);
    ce_hash_free(&inst->extension_points, alloc);

    CE_FREE(alloc, inst);
}

static void add_pass(void *inst,
                     void *pass,
                     uint64_t size) {
    render_graph_module_inst *module_inst = inst;

    ct_rg_pass_t0 *p = pass;
    p->size = size;

    ce_array_push_n(module_inst->pass, pass, size, _G.alloc);
}

static void module_on_setup(void *inst,
                            struct ct_rg_builder_t0 *builder) {
    render_graph_module_inst *module_inst = inst;

    const uint32_t pass_n = ce_array_size(module_inst->pass);
    for (int i = 0; i < pass_n;) {
        ct_rg_pass_t0 *pass = (ct_rg_pass_t0 *) &module_inst->pass[i];
        pass->on_setup(pass, builder);

        i += pass->size;
    }
}

static struct ct_rg_module_t0 *create_module();

typedef struct module_pass {
    ct_rg_pass_t0 pass;
    ct_rg_module_t0 *module;
} module_pass;

static void modulepass_on_setup(void *inst,
                                struct ct_rg_builder_t0 *builder) {
    module_pass *pass = inst;
    module_on_setup(pass->module->inst, builder);
}

static void module_add_module(void *inst,
                              struct ct_rg_module_t0 *new_module) {
    render_graph_module_inst *module_inst = inst;

    add_pass(inst, &(module_pass) {
            .module = new_module,
            .pass.on_setup = modulepass_on_setup,
    }, sizeof(module_pass));


    ce_array_push(module_inst->modules, new_module, _G.alloc);
}

struct ct_rg_module_t0 *add_extension_point(void *inst,
                                            uint64_t name) {
    render_graph_module_inst *module_inst = inst;

    ct_rg_module_t0 *m = create_module();

    module_add_module(inst, m);
    ce_hash_add(&module_inst->extension_points, name, (uint64_t) m, _G.alloc);

    return m;
}

struct ct_rg_module_t0 *get_extension_point(void *inst,
                                            uint64_t name) {
    render_graph_module_inst *module_inst = inst;

    uint64_t idx = ce_hash_lookup(&module_inst->extension_points, name, 0);
    return (ct_rg_module_t0 *) (idx);
}

static struct ct_rg_module_t0 *create_module() {
    ct_rg_module_t0 *obj = NULL;

    if (ce_array_any(_G.module_free)) {
        obj = ce_array_back(_G.module_free);
        ce_array_pop_back(_G.module_free);
    } else {
        obj = CE_ALLOC(_G.alloc, ct_rg_module_t0, sizeof(ct_rg_module_t0));
    }

    render_graph_module_inst *inst = NULL;
    if (ce_array_any(_G.module_inst_free)) {
        inst = ce_array_back(_G.module_inst_free);
        ce_array_pop_back(_G.module_inst_free);
    } else {
        inst = CE_ALLOC(_G.alloc, render_graph_module_inst,
                        sizeof(render_graph_module_inst));
    }

    *inst = (render_graph_module_inst) {};

    *obj = (ct_rg_module_t0) {
            .add_pass= add_pass,
            .get_extension_point = get_extension_point,
            .add_extension_point = add_extension_point,
            .add_module = module_add_module,
            .on_setup = module_on_setup,
            .inst = inst,
    };

    return obj;
}

static void destroy_module(ct_rg_module_t0 *module) {
    render_graph_module_inst *module_inst = module->inst;

    uint32_t n = ce_array_size(module_inst->modules);
    for (int i = 0; i < n; ++i) {
        destroy_module(module_inst->modules[i]);
    }

    ce_array_clean(module_inst->pass);
    ce_array_clean(module_inst->modules);
    ce_hash_clean(&module_inst->extension_points);

    ce_array_push(_G.module_free, module, _G.alloc);
    ce_array_push(_G.module_inst_free, module_inst, _G.alloc);
}