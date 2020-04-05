#include "celib/memory/allocator.h"

#include "celib/api.h"
#include "celib/memory/memory.h"
#include "celib/config.h"
#include "celib/module.h"
#include "celib/id.h"

#include <celib/core.h>
#include <celib/macros.h>
#include <celib/containers/buffer.h>
#include <celib/cdb.h>
#include <celib/log.h>
#include <celib/fs.h>
#include <celib/containers/hash.h>
#include <celib/containers/bagraph.h>
#include <cetech/asset/asset.h>

#include <cetech/machine/machine.h>

#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/static_module.h>
#include <cetech/asset_io/asset_io.h>
#include <cetech/ecs/ecs.h>
#include <celib/os/path.h>
#include <celib/os/time.h>
#include <cetech/metrics/metrics.h>
#include <celib/memory/memory_tracer.h>
#include "cetech/kernel/kernel.h"

#define CONFIG_MODULE_DIR \
     CE_ID64_0("module_dir", 0xa96daa49986032f4ULL)

static struct KernelGlobals {
    bool is_running;

    ce_ba_graph_t updateg;
    ce_hash_t update_map;

    ce_ba_graph_t initg;
    ce_hash_t init_map;
    ce_hash_t shutdown_map;

    ce_alloc_t0 *allocator;
} _G;

void register_api(struct ce_api_a0 *api);

const char *_platform() {
#if CE_PLATFORM_LINUX
    return "linux";
#elif defined(CETECH_WINDOWS)
    return "windows";
#elif CE_PLATFORM_OSX
    return "darwin";
#endif
    return NULL;
}

#define LOG_WHERE "kernel"

int init_config(int argc,
                const char **argv) {
    ce_config_a0->set_str(CONFIG_PLATFORM, _platform());
    ce_config_a0->set_str(CONFIG_NATIVE_PLATFORM, _platform());

    if (!ce_config_a0->parse_args(argc, argv)) {
        return 0;
    }

    const char *source_dir_str = ce_config_a0->read_str(CONFIG_SRC, "");
    char *source_config = NULL;
    ce_os_path_a0->join(&source_config, _G.allocator, 2, source_dir_str, "global.yml");
    ce_config_a0->from_file(ce_cdb_a0->db(), source_config, _G.allocator);

    ce_buffer_free(source_config, _G.allocator);

    return 1;
}


void application_quit() {
    _G.is_running = false;
}

static struct ct_kernel_a0 kernel_api = {
        .quit = application_quit,
};

struct ct_kernel_a0 *ct_kernel_a0 = &kernel_api;


bool cetech_kernel_init(int argc,
                        const char **argv) {
//    ce_log_a0->register_handler(ce_log_a0->stdout_yaml_handler, NULL);
    ce_log_a0->register_handler(ce_log_a0->stdout_handler, NULL);

    ce_init();

    ce_api_a0->add_api(CT_KERNEL_A0_STR, ct_kernel_a0, sizeof(kernel_api));

    _G = (struct KernelGlobals) {
            .allocator = ce_memory_a0->system,
    };

    init_config(argc, argv);

    init_static_modules();

    uint64_t root = ce_id_a0->id64("modules");

    const char *module_path = ce_config_a0->read_str(CONFIG_MODULE_DIR, "bin/darwin64");

    ce_fs_a0->map_root_dir(root, module_path, true);

    ce_module_a0->load_dirs(module_path);
    ce_config_a0->log_all();

    CE_INIT_API(ce_api_a0, ct_asset_a0);
    CE_INIT_API(ce_api_a0, ct_machine_a0);
    CE_INIT_API(ce_api_a0, ct_debugui_a0);
    CE_INIT_API(ce_api_a0, ct_renderer_a0);

    ce_cdb_a0->set_loader(ct_asset_a0->cdb_loader);
    return true;
}


int cetech_kernel_shutdown() {
    ce_log_a0->debug(LOG_WHERE, "Shutdown");
    ce_module_a0->unload_all();
    ce_shutdown();

    return 0;
}


void _init_config() {
    if (!ce_config_a0->exist(CONFIG_GAME)) {
        ce_config_a0->set_str(CONFIG_GAME, "editor");
    }
}


static void _build_update_graph(ce_ba_graph_t *sg) {
    ce_bag_clean(sg);
    ce_hash_clean(&_G.update_map);

    ce_api_entry_t0 it = ce_api_a0->first(CT_KERNEL_TASK_I0);
    while (it.api) {
        struct ct_kernel_task_i0 *i = (it.api);

        uint64_t name = i->name;

        ce_hash_add(&_G.update_map, name,
                    (uint64_t) i->update, _G.allocator);

        ce_bag_add(sg, name,
                   i->update_before.ptr, i->update_before.len,
                   i->update_after.ptr, i->update_after.len,
                   _G.allocator);

        it = ce_api_a0->next(it);
    }

    ce_bag_build(sg, _G.allocator);
}

static void _update(ce_ba_graph_t *sg,
                    float dt) {
    const uint64_t output_n = ce_array_size(sg->output);
    for (int k = 0; k < output_n; ++k) {
        ce_kernel_taks_update_t fce;
        fce = (ce_kernel_taks_update_t) ce_hash_lookup(&_G.update_map, sg->output[k], 0);
        fce(dt);
    }
}

static void _build_init_graph(ce_ba_graph_t *sg) {
    ce_bag_clean(sg);
    ce_hash_clean(&_G.init_map);
    ce_hash_clean(&_G.shutdown_map);

    ce_api_entry_t0 it = ce_api_a0->first(CT_KERNEL_TASK_I0);
    while (it.api) {
        struct ct_kernel_task_i0 *i = (it.api);

        if (!i->init) {
            it = ce_api_a0->next(it);
            continue;
        }

        uint64_t name = i->name;

        ce_hash_add(&_G.init_map, name,
                    (uint64_t) i->init, _G.allocator);

        if (i->shutdown) {
            ce_hash_add(&_G.shutdown_map, name,
                        (uint64_t) i->shutdown, _G.allocator);
        }

        ce_bag_add(sg, name,
                   i->init_before.ptr, i->init_before.len,
                   i->init_after.ptr, i->init_after.len,
                   _G.allocator);

        it = ce_api_a0->next(it);
    }

    ce_bag_build(sg, _G.allocator);
}

static void _init(ce_ba_graph_t *sg) {
    const uint64_t output_n = ce_array_size(sg->output);
    for (int k = 0; k < output_n; ++k) {
        ce_kernel_taks_init_t fce;
        fce = (ce_kernel_taks_init_t) ce_hash_lookup(&_G.init_map, sg->output[k], 0);
        fce();
    }
}

static void _shutdown(ce_ba_graph_t *sg) {
    const uint64_t output_n = ce_array_size(sg->output);
    for (int32_t k = output_n; k < 0; --k) {
        ce_kernel_taks_shutdown_t fce;
        fce = (ce_kernel_taks_shutdown_t) ce_hash_lookup(&_G.shutdown_map, sg->output[k], 0);

        if (!fce) {
            continue;
        }

        fce();
    }
}

static void _nop_update(float dt) {
}

static struct ct_kernel_task_i0 input_task = {
        .name = CT_INPUT_TASK,
        .update = _nop_update,
};


static void cetech_kernel_start() {
    ce_api_a0->add_impl(CT_KERNEL_TASK_I0_STR, &input_task, sizeof(input_task));

    _init_config();

    ct_asset_a0->compile_all();

    if (ce_config_a0->read_uint(CONFIG_COMPILE, 0)) {
        return;
    }

    _build_init_graph(&_G.initg);
    _init(&_G.initg);

    _G.is_running = 1;

    const uint64_t fq = ce_os_time_a0->perf_freq();
    uint64_t last_tick = ce_os_time_a0->perf_counter();

    ce_mem_tracer_t0 *t = ce_memory_a0->system->vt->memory_tracer(ce_memory_a0->system->inst);

    ct_metrics_a0->reg_float_metric("dt");
    ct_metrics_a0->reg_float_metric("memory.system");

    while (_G.is_running) {
        ce_module_a0->do_reload();

        uint64_t now_ticks = ce_os_time_a0->perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        ct_metrics_a0->begin();
        ct_metrics_a0->set_float(ce_id_a0->id64("dt"), dt);
        ct_metrics_a0->set_float(ce_id_a0->id64("memory.system"),
                                 t->vt->allocated_size(t->inst) * 0.000001);

        _build_update_graph(&_G.updateg);
        _update(&_G.updateg, dt);

        ce_cdb_a0->tick();

        ct_metrics_a0->end();
    }

    _shutdown(&_G.initg);
}

int main(int argc,
         const char **argv) {

    if (cetech_kernel_init(argc, argv)) {
        cetech_kernel_start();
    }

    return cetech_kernel_shutdown();
}
