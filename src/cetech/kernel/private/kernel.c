#include "celib/allocator.h"

#include "celib/api_system.h"
#include <celib/os.h>
#include "cetech/kernel/kernel.h"
#include "celib/memory.h"
#include "celib/config.h"
#include "celib/module.h"
#include "celib/hashlib.h"
#include "celib/private/api_private.h"

#include <celib/ebus.h>

#include <celib/core.h>
#include <celib/buffer.inl>
#include <celib/task.h>
#include <cetech/resource/resource.h>
#include <cetech/resource/package.h>
#include <cetech/machine/machine.h>
#include <cetech/gfx/debugui.h>
#include <cetech/gfx/renderer.h>


#include <cetech/static_module.h>
#include <celib/log.h>
#include <cetech/game_system/game_system.h>
#include <celib/fs.h>


static struct KernelGlobals {
    uint64_t config_object;
    int is_running;
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

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(object);
    ce_cdb_a0->set_str(writer, CONFIG_PLATFORM, _platform());
    ce_cdb_a0->set_str(writer, CONFIG_NATIVE_PLATFORM, _platform());
    ce_cdb_a0->set_str(writer, CONFIG_BUILD, "build");
    ce_cdb_a0->write_commit(writer);

    if (!ce_config_a0->parse_args(argc, argv)) {
        return 0;
    }

    const char *build_dir_str = ce_cdb_a0->read_str(object, CONFIG_BUILD, "");
    char *build_dir = NULL;
    ce_os_a0->path->join(&build_dir, _G.allocator, 2,
                         build_dir_str,
                         ce_cdb_a0->read_str(object, CONFIG_NATIVE_PLATFORM,
                                             ""));

    char *build_config = NULL;
    ce_os_a0->path->join(&build_config, _G.allocator, 2, build_dir,
                         "global.config");

    const char *source_dir_str = ce_cdb_a0->read_str(object, CONFIG_SRC, "");
    char *source_config = NULL;
    ce_os_a0->path->join(&source_config, _G.allocator, 2, source_dir_str,
                         "global.config");

    if (ce_cdb_a0->read_uint64(object, CONFIG_COMPILE, 0)) {
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

int cetech_kernel_init(int argc,
                       const char **argv) {
    ce_log_a0->register_handler(ce_log_a0->stdout_yaml_handler, NULL);

    ce_init();

    struct ce_api_a0 *api = ce_api_a0;

    _G = (struct KernelGlobals) {
            .allocator = ce_memory_a0->system,
            .config_object  = ce_config_a0->obj(),
    };

    init_config(argc, argv, ce_config_a0->obj());

    CE_ADD_STATIC_MODULE(builddb);
    CE_ADD_STATIC_MODULE(resourcesystem);
    CE_ADD_STATIC_MODULE(resourcecompiler);

    CE_INIT_API(api, ce_ebus_a0);
    ce_ebus_a0->create_ebus("kernel", KERNEL_EBUS);

    init_static_modules();

    uint64_t root = ce_id_a0->id64("modules");
    const char *module_path = ce_cdb_a0->read_str(ce_config_a0->obj(),
                                                  CONFIG_MODULE_DIR,
                                                  "bin/darwin64");

    ce_fs_a0->map_root_dir(root, module_path, true);

    ce_module_a0->load_dirs(module_path);
    ce_config_a0->log_all();


    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_os_a0);
    CE_INIT_API(api, ct_package_a0);
    CE_INIT_API(api, ct_machine_a0);
    CE_INIT_API(api, ct_debugui_a0);
    CE_INIT_API(api, ct_renderer_a0);

#if defined(CETECH_DEVELOP)
    ct_resource_a0->set_autoload(true);
#else
    ct_resource_a0->set_autoload(0);
#endif

    return 1;
}


int cetech_kernel_shutdown() {
    ce_log_a0->debug(LOG_WHERE, "Shutdown");
    ce_module_a0->unload_all();
    ce_shutdown();

    return 1;
}

void application_quit() {
    _G.is_running = 0;
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

static void _boot_stage() {
    const char *boot_pkg_str = ce_cdb_a0->read_str(_G.config_object,
                                                   CONFIG_BOOT_PKG, "");
    uint64_t boot_pkg = ce_id_a0->id64(boot_pkg_str);


    uint64_t core_pkg = ce_id_a0->id64("core/core");
    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_resource_a0->load_now(PACKAGE_TYPE, resources, 2);

    struct ce_task_counter_t *boot_pkg_cnt = ct_package_a0->load(boot_pkg);
    ct_package_a0->flush(boot_pkg_cnt);

    struct ce_task_counter_t *core_pkg_cnt = ct_package_a0->load(core_pkg);
    ct_package_a0->flush(core_pkg_cnt);
}

static void _boot_unload() {
    const char *boot_pkg_str = ce_cdb_a0->read_str(_G.config_object,
                                                   CONFIG_BOOT_PKG, "");
    uint64_t boot_pkg = ce_id_a0->id64(boot_pkg_str);

    uint64_t core_pkg = ce_id_a0->id64("core/core");

    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_package_a0->unload(boot_pkg);
    ct_package_a0->unload(core_pkg);

    ct_resource_a0->unload(PACKAGE_TYPE, resources, 2);
}


static void on_quit(uint64_t event) {
    CE_UNUSED(event)

    application_quit();
}

static void cetech_kernel_start() {

    _init_config();

    if (ce_cdb_a0->read_uint64(_G.config_object, CONFIG_COMPILE, 0)) {
        ct_resource_a0->compiler_compile_all();

        if (!ce_cdb_a0->read_uint64(_G.config_object, CONFIG_CONTINUE, 0)) {
            return;
        }
    }

    _boot_stage();

    ce_ebus_a0->connect(KERNEL_EBUS, KERNEL_QUIT_EVENT, on_quit, 0);

    uint64_t event = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                              KERNEL_INIT_EVENT);

    ce_ebus_a0->broadcast(KERNEL_EBUS, event);

    _G.is_running = 1;

    const uint64_t fq = ce_os_a0->time->perf_freq();
    uint64_t last_tick = ce_os_a0->time->perf_counter();

    while (_G.is_running) {
        uint64_t now_ticks = ce_os_a0->time->perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        ce_ebus_a0->begin_frame();

        uint64_t event;
        event = ce_cdb_a0->create_object(ce_cdb_a0->db(), KERNEL_UPDATE_EVENT);
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(event);
        ce_cdb_a0->set_float(w, KERNEL_EVENT_DT, dt);
        ce_cdb_a0->write_commit(w);

        ce_ebus_a0->broadcast(KERNEL_EBUS, event);

        ce_cdb_a0->gc();
    }

    event = ce_cdb_a0->create_object(ce_cdb_a0->db(), KERNEL_SHUTDOWN_EVENT);
    ce_ebus_a0->broadcast(KERNEL_EBUS, event);

    ce_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_QUIT_EVENT, on_quit);

    _boot_unload();
}

int main(int argc,
         const char **argv) {

    if (cetech_kernel_init(argc, argv)) {
        cetech_kernel_start();
    }

    return cetech_kernel_shutdown();
}
