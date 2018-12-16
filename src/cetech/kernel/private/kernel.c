#include "celib/allocator.h"
#include <celib/os.h>
#include "celib/api_system.h"
#include "celib/memory.h"
#include "celib/config.h"
#include "celib/module.h"
#include "celib/hashlib.h"
#include "celib/private/api_private.h"
#include <celib/ebus.h>
#include <celib/core.h>
#include <celib/buffer.inl>
#include <celib/cdb.h>
#include <celib/task.h>
#include <celib/log.h>
#include <celib/fs.h>
#include <celib/bagraph.h>
#include <cetech/resource/resource.h>


#include <cetech/machine/machine.h>
#include <cetech/debugui/debugui.h>
#include <cetech/renderer/renderer.h>
#include <cetech/game/game_system.h>
#include <cetech/static_module.h>
#include <cetech/resource/builddb.h>
#include <cetech/resource/resource_compiler.h>
#include <cetech/asset/sourcedb.h>
#include <cetech/ecs/ecs.h>
#include <stdlib.h>
#include <celib/ydb.h>
#include "cetech/kernel/kernel.h"

static struct KernelGlobals {
    uint64_t config_object;
    bool is_running;

    struct ce_ba_graph updateg;
    struct ce_hash_t update_map;

    struct ce_alloc *allocator;
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
                const char **argv,
                uint64_t object) {

//    {
//        uint64_t obj1 = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
//        uint64_t obj2 = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
//        uint64_t obj3 = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
//        uint64_t obj4 = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
//        uint64_t obj5 = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
//
//        ce_cdb_obj_o *writer1 = ce_cdb_a0->write_begin(obj1);
//        ce_cdb_obj_o *writer2 = ce_cdb_a0->write_begin(obj2);
//        ce_cdb_obj_o *writer3 = ce_cdb_a0->write_begin(obj3);
//        ce_cdb_obj_o *writer4 = ce_cdb_a0->write_begin(obj4);
//        ce_cdb_a0->set_subobject(writer1, 1, obj2);
//        ce_cdb_a0->write_commit(writer1);
//
//        ce_cdb_a0->set_subobject(writer2, 1, obj3);
//        ce_cdb_a0->write_commit(writer2);
//
//        ce_cdb_a0->set_subobject(writer3, 1, obj4);
//        ce_cdb_a0->write_commit(writer3);
//
//        ce_cdb_a0->set_subobject(writer4, 1, obj5);
//        ce_cdb_a0->write_commit(writer4);
//
//        uint64_t parent1 = ce_cdb_a0->parent(obj2);
////        CE_ASSERT(LOG_WHERE, parent1 == obj1);
//    }

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(object);
    ce_cdb_a0->set_str(writer, CONFIG_PLATFORM, _platform());
    ce_cdb_a0->set_str(writer, CONFIG_NATIVE_PLATFORM, _platform());
    ce_cdb_a0->set_str(writer, CONFIG_BUILD, "build");
    ce_cdb_a0->write_commit(writer);

    if (!ce_config_a0->parse_args(argc, argv)) {
        return 0;
    }

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(object);

    const char *build_dir_str = ce_cdb_a0->read_str(reader, CONFIG_BUILD, "");
    char *build_dir = NULL;
    ce_os_a0->path->join(&build_dir, _G.allocator, 2,
                         build_dir_str,
                         ce_cdb_a0->read_str(reader, CONFIG_NATIVE_PLATFORM,
                                             ""));

    char *build_config = NULL;
    ce_os_a0->path->join(&build_config, _G.allocator, 2, build_dir,
                         "global.yml");

    const char *source_dir_str = ce_cdb_a0->read_str(reader, CONFIG_SRC, "");
    char *source_config = NULL;
    ce_os_a0->path->join(&source_config, _G.allocator, 2, source_dir_str,
                         "global.yml");

    if (ce_cdb_a0->read_uint64(reader, CONFIG_COMPILE, 0)) {
        ce_os_a0->path->make_path(build_dir);
        ce_os_a0->path->copy_file(_G.allocator, source_config, build_config);
    }

    ce_config_a0->load_from_yaml_file(build_config, _G.allocator);

    ce_buffer_free(source_config, _G.allocator);
    ce_buffer_free(build_config, _G.allocator);
    ce_buffer_free(build_dir, _G.allocator);

    if (!ce_config_a0->parse_args(argc, argv)) {
        return 0;
    }

    return 1;
}


bool cetech_kernel_init(int argc,
                        const char **argv) {
//    ce_log_a0->register_handler(ce_log_a0->stdout_yaml_handler, NULL);
    ce_log_a0->register_handler(ce_log_a0->stdout_handler, NULL);

    ce_init();


    struct ce_api_a0 *api = ce_api_a0;

    _G = (struct KernelGlobals) {
            .allocator = ce_memory_a0->system,
            .config_object  = ce_config_a0->obj(),
    };

    init_config(argc, argv, ce_config_a0->obj());


    CE_INIT_API(api, ce_ebus_a0);
    ce_ebus_a0->create_ebus(KERNEL_EBUS);

    init_static_modules();

    uint64_t root = ce_id_a0->id64("modules");

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_config_a0->obj());

    const char *module_path = ce_cdb_a0->read_str(reader,
                                                  CONFIG_MODULE_DIR,
                                                  "bin/darwin64");

    ce_fs_a0->map_root_dir(root, module_path, true);

    ce_module_a0->load_dirs(module_path);
    ce_config_a0->log_all();

    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_os_a0);
    CE_INIT_API(api, ct_machine_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ct_renderer_a0);

    return true;
}


int cetech_kernel_shutdown() {
    ce_log_a0->debug(LOG_WHERE, "Shutdown");
    ce_module_a0->unload_all();
    ce_shutdown();

    return 1;
}

void application_quit() {
    _G.is_running = false;
}


void _init_config() {
    _G.config_object = ce_config_a0->obj();

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(_G.config_object);

    if (!ce_cdb_a0->prop_exist(_G.config_object, CONFIG_BOOT_PKG)) {
        ce_cdb_a0->set_str(writer, CONFIG_BOOT_PKG, "boot");
    }

    if (!ce_cdb_a0->prop_exist(_G.config_object, CONFIG_GAME)) {
        ce_cdb_a0->set_str(writer, CONFIG_GAME, "editor");
    }

    if (!ce_cdb_a0->prop_exist(_G.config_object, CONFIG_DAEMON)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_DAEMON, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config_object, CONFIG_COMPILE)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_COMPILE, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config_object, CONFIG_CONTINUE)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_CONTINUE, 0);
    }

    if (!ce_cdb_a0->prop_exist(_G.config_object, CONFIG_WAIT)) {
        ce_cdb_a0->set_uint64(writer, CONFIG_WAIT, 0);
    }

    ce_cdb_a0->write_commit(writer);
}

static void on_quit(uint64_t type,
                    void *event) {
    CE_UNUSED(event)
    application_quit();
}

static void _build_update_graph(struct ce_ba_graph *sg) {
    ce_bag_clean(sg);
    ce_hash_clean(&_G.update_map);

    struct ce_api_entry it = ce_api_a0->first(KERNEL_TASK_INTERFACE);
    while (it.api) {
        struct ct_kernel_task_i0 *i = (it.api);

        uint64_t name = i->name();

        ce_hash_add(&_G.update_map, name,
                    (uint64_t) i->update, _G.allocator);

        uint64_t before_n = 0;
        const uint64_t *before = NULL;
        if (i->update_before) {
            before = i->update_before(&before_n);
        }

        uint64_t after_n = 0;
        const uint64_t *after;
        if (i->update_after) {
            after = i->update_after(&after_n);
        }

        ce_bag_add(&_G.updateg, name,
                   before, before_n,
                   after, after_n, _G.allocator);

        it = ce_api_a0->next(it);
    }

    ce_bag_build(sg, _G.allocator);
}

static void _update(struct ce_ba_graph *sg,
                    float dt) {
    const uint64_t output_n = ce_array_size(sg->output);
    for (int k = 0; k < output_n; ++k) {
        ce_kernel_taks_update_t fce;
        fce = (ce_kernel_taks_update_t) ce_hash_lookup(&_G.update_map,
                                                       sg->output[k], 0);
        fce(dt);
    }
}

static void _nop_update(float dt) {
}

static uint64_t input_task_name() {
    return CT_INPUT_TASK;
}

static struct ct_kernel_task_i0 input_task = {
        .name = input_task_name,
        .update = _nop_update,
};

static void cetech_kernel_start() {
    ce_api_a0->register_api("ct_kernel_task_i0", &input_task);

    _init_config();

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(_G.config_object);

    if (ce_cdb_a0->read_uint64(reader, CONFIG_COMPILE, 0)) {
        ct_resource_compiler_a0->compile_all();

        if (!ce_cdb_a0->read_uint64(reader, CONFIG_CONTINUE, 0)) {
            return;
        }
    }

#if 1
    char *buf = NULL;
    uint64_t r = ct_sourcedb_a0->get((struct ct_resource_id) {
            .type = ENTITY_RESOURCE_ID,
            .name = ce_id_a0->id64("content/level2")
    });
    ce_cdb_a0->dump_str(&buf, r, 0);
    ce_log_a0->debug(LOG_WHERE, "%s", buf);

    ce_buffer_clear(buf);
    r = ct_sourcedb_a0->get((struct ct_resource_id) {
            .type = ce_id_a0->id64("material"),
            .name = ce_id_a0->id64("content/material2")
    });
    ce_cdb_a0->dump_str(&buf, r, 0);
    ce_log_a0->debug(LOG_WHERE, "%s", buf);
// abort();
#endif

    ce_ebus_a0->connect(KERNEL_EBUS, KERNEL_QUIT_EVENT, on_quit, 0);

    ce_ebus_a0->broadcast(KERNEL_EBUS, KERNEL_INIT_EVENT, NULL, 0);

    _G.is_running = 1;

    const uint64_t fq = ce_os_a0->time->perf_freq();
    uint64_t last_tick = ce_os_a0->time->perf_counter();

    while (_G.is_running) {
        uint64_t now_ticks = ce_os_a0->time->perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        _build_update_graph(&_G.updateg);
        _update(&_G.updateg, dt);

        ce_ebus_a0->gc();
        ce_cdb_a0->gc();
    }

    ce_ebus_a0->broadcast(KERNEL_EBUS, KERNEL_SHUTDOWN_EVENT, NULL, 0);

    ce_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_QUIT_EVENT, on_quit);
}

int main(int argc,
         const char **argv) {

    if (cetech_kernel_init(argc, argv)) {
        cetech_kernel_start();
    }

    return cetech_kernel_shutdown();
}
