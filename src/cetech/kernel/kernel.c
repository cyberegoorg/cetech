#include "cetech/kernel/memory/allocator.h"

#include "cetech/kernel/api/api_system.h"
#include "cetech/kernel/os/path.h"
#include "kernel.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/config/config.h"
#include "cetech/kernel/module/module.h"
#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/os/time.h"
#include "cetech/kernel/log/private/log_system_private.h"
#include "cetech/kernel/memory/private/memory_private.h"
#include "cetech/kernel/api/private/api_private.h"
#include "cetech/kernel/memory/private/allocator_core_private.h"

#include <cetech/kernel/ebus/ebus.h>
#include <cetech/kernel/cdb/cdb.h>

#include <cetech/kernel/core.h>
#include <cetech/kernel/containers/buffer.h>
#include <cetech/kernel/task/task.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/engine/resource/package.h>
#include <cetech/kernel/task/task.h>
#include <cetech/engine/machine/machine.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/renderer.h>

CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_time_a0);
CETECH_DECL_API(ct_package_a0);
CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_renderer_a0);

#include <cetech/static_module.h>


static struct KernelGlobals {
    struct ct_cdb_obj_t *config_object;
    int is_running;
    struct ct_alloc *allocator;
} _G;

void register_api(struct ct_api_a0 *api);

const char *_platform() {
#if defined(CETECH_LINUX)
    return "linux";
#elif defined(CETECH_WINDOWS)
    return "windows";
#elif defined(CETECH_DARWIN)
    return "darwin";
#endif
    return NULL;
}

#define LOG_WHERE "kernel"

#define CONFIG_PLATFORM CT_ID64_0(CONFIG_PLATFORM_ID)
#define CONFIG_NATIVE_PLATFORM CT_ID64_0(CONFIG_NATIVE_PLATFORM_ID)

#define CONFIG_BUILD CT_ID64_0(CONFIG_BUILD_ID)
#define CONFIG_SRC CT_ID64_0(CONFIG_SRC_ID)
#define CONFIG_COMPILE CT_ID64_0(CONFIG_COMPILE_ID)

int init_config(int argc,
                const char **argv,
                struct ct_cdb_obj_t *object) {

    struct ct_cdb_obj_t *writer = ct_cdb_a0.write_begin(object);
    ct_cdb_a0.set_string(writer, CONFIG_PLATFORM, _platform());
    ct_cdb_a0.set_string(writer, CONFIG_NATIVE_PLATFORM, _platform());
    ct_cdb_a0.set_string(writer, CONFIG_BUILD, "build");
    ct_cdb_a0.write_commit(writer);

    if (!ct_config_a0.parse_args(argc, argv)) {
        return 0;
    }

    struct ct_alloc *a = ct_memory_a0.main_allocator();

    const char *build_dir_str = ct_cdb_a0.read_str(object, CONFIG_BUILD, "");
    char *build_dir = NULL;
    ct_path_a0.join(&build_dir, a, 2,
                    build_dir_str,
                    ct_cdb_a0.read_str(object, CONFIG_NATIVE_PLATFORM, ""));

    char *build_config = NULL;
    ct_path_a0.join(&build_config, a, 2, build_dir, "global.config");

    const char *source_dir_str = ct_cdb_a0.read_str(object, CONFIG_SRC, "");
    char *source_config = NULL;
    ct_path_a0.join(&source_config, a, 2, source_dir_str, "global.config");

    if (ct_cdb_a0.read_uint32(object, CONFIG_COMPILE, 0)) {
        ct_path_a0.make_path(build_dir);
        ct_path_a0.copy_file(a, source_config, build_config);
    }

    ct_config_a0.load_from_yaml_file(build_config, a);

    ct_buffer_free(source_config, a);
    ct_buffer_free(build_config, a);
    ct_buffer_free(build_dir, a);

    if (!ct_config_a0.parse_args(argc, argv)) {
        return 0;
    }

    return 1;
}

int cetech_kernel_init(int argc,
                       const char **argv) {
    corelib_init();
    struct ct_api_a0 *api = api_v0();

    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_path_a0);
    CETECH_GET_API(api, ct_config_a0);
    CETECH_GET_API(api, ct_module_a0);
    CETECH_GET_API(api, ct_cdb_a0);
    CETECH_GET_API(api, ct_hashlib_a0);
    CETECH_GET_API(api, ct_task_a0);

    _G = (struct KernelGlobals) {
            .allocator = ct_memory_a0.main_allocator(),
            .config_object  = ct_config_a0.config_object(),
    };

    init_config(argc, argv, ct_config_a0.config_object());

    CETECH_ADD_STATIC_MODULE(resourcesystem);
    CETECH_ADD_STATIC_MODULE(resourcecompiler);

    CETECH_GET_API(api, ct_ebus_a0);
    ct_ebus_a0.create_ebus("kernel", KERNEL_EBUS);

    init_static_modules();

    ct_module_a0.load_dirs();
    ct_config_a0.log_all();


    CETECH_GET_API(api, ct_resource_a0);
    CETECH_GET_API(api, ct_time_a0);
    CETECH_GET_API(api, ct_package_a0);

    CETECH_GET_API(api, ct_machine_a0);
    CETECH_GET_API(api, ct_debugui_a0);
    CETECH_GET_API(api, ct_renderer_a0);


#if defined(CETECH_DEVELOP)
    ct_resource_a0.set_autoload(true);
#else
    ct_resource_a0.set_autoload(0);
#endif


    return 1;
}


int cetech_kernel_shutdown() {
    ct_log_a0.debug(LOG_WHERE, "Shutdown");
    ct_module_a0.unload_all();
    corelib_shutdown();

    return 1;
}

void application_quit() {
    _G.is_running = 0;
}

#define CONFIG_BOOT_PKG CT_ID64_0(CONFIG_BOOT_PKG_ID)
#define CONFIG_DAEMON CT_ID64_0(CONFIG_DAEMON_ID)
#define CONFIG_COMPILE CT_ID64_0(CONFIG_COMPILE_ID)
#define CONFIG_CONTINUE CT_ID64_0(CONFIG_CONTINUE_ID)
#define CONFIG_WAIT CT_ID64_0(CONFIG_WAIT_ID)
#define CONFIG_GAME CT_ID64_0(CONFIG_GAME_ID)

void _init_config() {
    _G.config_object = ct_config_a0.config_object();

    struct ct_cdb_obj_t *writer = ct_cdb_a0.write_begin(_G.config_object);

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_BOOT_PKG)) {
        ct_cdb_a0.set_string(writer, CONFIG_BOOT_PKG, "boot");
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_GAME)) {
        ct_cdb_a0.set_string(writer, CONFIG_GAME, "playground");
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_DAEMON)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_DAEMON, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_COMPILE)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_COMPILE, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_CONTINUE)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_CONTINUE, 0);
    }

    if (!ct_cdb_a0.prop_exist(_G.config_object, CONFIG_WAIT)) {
        ct_cdb_a0.set_uint32(writer, CONFIG_WAIT, 0);
    }

    ct_cdb_a0.write_commit(writer);
}

static void _boot_stage() {
    const char *boot_pkg_str = ct_cdb_a0.read_str(_G.config_object,
                                                  CONFIG_BOOT_PKG, "");
    uint32_t boot_pkg = CT_ID32_0(boot_pkg_str);
    uint32_t pkg = CT_ID32_0("package");

    uint32_t core_pkg = CT_ID32_0("core/core");
    uint32_t resources[] = {core_pkg, boot_pkg};

    ct_resource_a0.load_now(pkg, resources, 2);

    struct ct_task_counter_t *boot_pkg_cnt = ct_package_a0.load(boot_pkg);
    ct_package_a0.flush(boot_pkg_cnt);

    struct ct_task_counter_t *core_pkg_cnt = ct_package_a0.load(core_pkg);
    ct_package_a0.flush(core_pkg_cnt);
}

static void _boot_unload() {
    const char *boot_pkg_str = ct_cdb_a0.read_str(_G.config_object,
                                                  CONFIG_BOOT_PKG, "");
    uint32_t boot_pkg = CT_ID32_0(boot_pkg_str);

    uint32_t core_pkg = CT_ID32_0("core/core");
    uint32_t pkg = CT_ID32_0("package");

    uint32_t resources[] = {core_pkg, boot_pkg};

    ct_package_a0.unload(boot_pkg);
    ct_package_a0.unload(core_pkg);

    ct_resource_a0.unload(pkg, resources, 2);
}


static void on_quit(uint32_t ebus, void* event) {
    application_quit();
}

void application_start() {
    _init_config();

    if (ct_cdb_a0.read_uint32(_G.config_object, CONFIG_COMPILE, 0)) {
        ct_resource_a0.compiler_compile_all();

        if (!ct_cdb_a0.read_uint32(_G.config_object, CONFIG_CONTINUE, 0)) {
            return;
        }
    }

    _boot_stage();

    ct_ebus_a0.connect(KERNEL_EBUS, KERNEL_QUIT_EVENT, on_quit, 0);

    ct_ebus_a0.broadcast(KERNEL_EBUS, KERNEL_INIT_EVENT, NULL, 0);

    _G.is_running = 1;

    uint64_t fq = ct_time_a0.perf_freq();
    uint64_t last_tick = ct_time_a0.perf_counter();
    while (_G.is_running) {
        ct_ebus_a0.begin_frame();

        uint64_t now_ticks = ct_time_a0.perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        ct_machine_a0.update(dt);

        struct ct_app_update_ev ev = {.dt=dt};
        ct_ebus_a0.broadcast(KERNEL_EBUS, KERNEL_UPDATE_EVENT, &ev, sizeof(ev));

        ct_cdb_a0.gc();
    }

    ct_ebus_a0.broadcast(KERNEL_EBUS, KERNEL_SHUTDOWN_EVENT, NULL, 0);

    ct_ebus_a0.disconnect(KERNEL_EBUS, KERNEL_QUIT_EVENT, on_quit);

    _boot_unload();
}

static void cetech_kernel_start(){
    application_start();
}

int main(int argc,
         const char **argv) {

    if (cetech_kernel_init(argc, argv)) {
        cetech_kernel_start();
    }

    return cetech_kernel_shutdown();
}
