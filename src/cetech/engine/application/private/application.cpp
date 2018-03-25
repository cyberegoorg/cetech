//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <cetech/core/macros.h>
#include <cetech/core/containers/container_types.inl>

#include <cetech/core/containers/map.inl>
#include <cetech/core/ebus/ebus.h>
#include <cetech/engine/application/application.h>
#include <cetech/core/os/watchdog.h>
#include <cetech/core/fs/fs.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/api/api_system.h>
#include <cetech/engine/resource/package.h>
#include <cetech/core/task/task.h>
#include <cetech/core/config/config.h>
#include <cetech/core/os/time.h>
#include <cetech/core/os/path.h>
#include <cetech/core/log/log.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/memory/memory.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/core/module/module.h>
#include <cetech/engine/kernel/kernel.h>

// TODO: shit , app == module?
#include <cetech/engine/machine/machine.h>
#include <cetech/engine/renderer/renderer.h>

#include <cetech/core/containers/array.h>
#include <cetech/core/containers/hash.h>
#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/debugui/debugui.h>


CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_package_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_time_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_watchdog_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_fs_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_ebus_a0);

//==============================================================================
// Definess
//==============================================================================

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

static struct ApplicationGlobals {
    ct_cdb_obj_t *config_object;

    int is_running;
    ct_alloc *allocator;
} _G;


//==============================================================================
// Interface
//==============================================================================

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

    ct_cdb_obj_t *writer = ct_cdb_a0.write_begin(_G.config_object);

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

    ct_task_counter_t *boot_pkg_cnt = ct_package_a0.load(boot_pkg);
    ct_package_a0.flush(boot_pkg_cnt);

    ct_task_counter_t *core_pkg_cnt = ct_package_a0.load(core_pkg);
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

extern "C" void application_start() {
    _init_config();

    if (ct_cdb_a0.read_uint32(_G.config_object, CONFIG_COMPILE, 0)) {
        ct_resource_a0.compiler_compile_all();

        if (!ct_cdb_a0.read_uint32(_G.config_object, CONFIG_CONTINUE, 0)) {
            return;
        }
    }

    _boot_stage();

    ct_ebus_a0.connect(APPLICATION_EBUS, APP_QUIT_EVENT, on_quit, 0);

    ct_ebus_a0.broadcast(APPLICATION_EBUS, APP_INI_EVENT, NULL, 0);

    ct_ebus_a0.broadcast(APPLICATION_EBUS, APP_GAME_INIT_EVENT, NULL, 0);

    _G.is_running = 1;

    uint64_t fq = ct_time_a0.perf_freq();
    uint64_t last_tick = ct_time_a0.perf_counter();
    while (_G.is_running) {
        ct_ebus_a0.begin_frame();

        uint64_t now_ticks = ct_time_a0.perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        ct_machine_a0.update(dt);

        ct_app_update_ev ev = {.dt=dt};
        ct_ebus_a0.broadcast(APPLICATION_EBUS, APP_UPDATE_EVENT, &ev, sizeof(ev));

        ct_ebus_a0.broadcast(APPLICATION_EBUS, APP_GAME_UPDATE_EVENT, &ev, sizeof(ev));

        if (!ct_cdb_a0.read_uint32(_G.config_object, CONFIG_DAEMON, 0)) {
            ct_debugui_a0 CETECH_GET_API(&ct_api_a0, ct_debugui_a0);
            ct_debugui_a0.render(255);
            ct_renderer_a0.render();
        }

        ct_cdb_a0.gc();
    }

    ct_ebus_a0.broadcast(APPLICATION_EBUS, APP_GAME_SHUTDOWN_EVENT, NULL, 0);

    ct_ebus_a0.broadcast(APPLICATION_EBUS, APP_SHUTDOWN_EVENT, NULL, 0);

    _boot_unload();
}

static ct_app_a0 a0 = {
        .quit = application_quit,
        .start = application_start,
};

void app_init(struct ct_api_a0 *api) {
    api->register_api("ct_app_a0", &a0);

    _G.allocator = ct_memory_a0.main_allocator();

    ct_ebus_a0.create_ebus(APPLICATION_EBUS_NAME, APPLICATION_EBUS);

#if defined(CETECH_DEVELOP)
    ct_resource_a0.set_autoload(true);
#else
    ct_resource_a0.set_autoload(0);
#endif
}

CETECH_MODULE_DEF(
        application,
        {
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_time_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);

            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_package_a0);
            CETECH_GET_API(api, ct_task_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_module_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_ydb_a0);

            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_ebus_a0);

            ct_api_a0 = *api;
        },
        {
            CT_UNUSED(reload);

            app_init(api);
        },
        {
            CT_UNUSED(api, reload);
        }
)
