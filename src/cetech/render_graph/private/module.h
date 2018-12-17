struct render_graph_module_inst {
    uint8_t *pass;
    struct ct_rg_module **modules;
    struct ce_hash_t extension_points;
};

static void add_pass(void *inst,
                     void *pass,
                     uint64_t size) {
    struct ct_rg_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;

    struct ct_rg_pass *p = pass;
    p->size = size;

    ce_array_push_n(module_inst->pass, pass, size, _G.alloc);
}

static void module_on_setup(void *inst,
                            struct ct_rg_builder *builder) {
    struct ct_rg_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;

    const uint32_t pass_n = ce_array_size(module_inst->pass);
    for (int i = 0; i < pass_n;) {
        struct ct_rg_pass *pass = (struct ct_rg_pass *) &module_inst->pass[i];
        pass->on_setup(pass, builder);

        i += pass->size;
    }
}

static struct ct_rg_module *create_module();

struct module_pass {
    struct ct_rg_pass pass;
    struct ct_rg_module *module;
};

static void modulepass_on_setup(void *inst,
                                struct ct_rg_builder *builder) {
    struct module_pass *pass = inst;
    module_on_setup(pass->module, builder);
}

static void module_add_module(void *inst,
                              struct ct_rg_module *new_module) {
    struct ct_rg_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;

    add_pass(module, &(struct module_pass) {
            .module = new_module,
            .pass.on_setup = modulepass_on_setup,
    }, sizeof(struct module_pass));


    ce_array_push(module_inst->modules, new_module, _G.alloc);
}

void add_extension_point(void *inst,
                         uint64_t name) {
    struct ct_rg_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;

    struct ct_rg_module *m = create_module();

    module_add_module(module, m);
    ce_hash_add(&module_inst->extension_points, name, (uint64_t) m, _G.alloc);
}

struct ct_rg_module *get_extension_point(void *inst,
                                                   uint64_t name) {
    struct ct_rg_module *module = inst;
    struct render_graph_module_inst *module_inst = module->inst;

    uint64_t idx = ce_hash_lookup(&module_inst->extension_points, name, 0);
    return (struct ct_rg_module *) (idx);
}

static void destroy_module(struct ct_rg_module *module) {
    struct render_graph_module_inst *module_inst = module->inst;

    uint32_t n = ce_array_size(module_inst->modules);
    for (int i = 0; i < n; ++i) {
        destroy_module(module_inst->modules[i]);
    }

    ce_array_free(module_inst->pass, _G.alloc);
    ce_array_free(module_inst->modules, _G.alloc);
    ce_hash_free(&module_inst->extension_points, _G.alloc);

    CE_FREE(_G.alloc, module->inst);
    CE_FREE(_G.alloc, module);
}

static struct ct_rg_module *create_module() {
    struct ct_rg_module *obj = CE_ALLOC(_G.alloc,
                                                  struct ct_rg_module,
                                                  sizeof(struct ct_rg_module));

    struct render_graph_module_inst *inst = CE_ALLOC(_G.alloc,
                                                     struct render_graph_module_inst,
                                                     sizeof(struct render_graph_module_inst));
    *inst = (struct render_graph_module_inst){};

    *obj = (struct ct_rg_module) {
            .add_pass= add_pass,
            .get_extension_point = get_extension_point,
            .add_extension_point = add_extension_point,
            .add_module = module_add_module,
            .on_setup = module_on_setup,
            .inst = inst,
    };

    return obj;
}

