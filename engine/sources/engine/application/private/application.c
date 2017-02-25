//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <celib/time/time.h>
#include <celib/window/types.h>
#include <celib/window/window.h>
#include <celib/string/stringid.h>
#include <celib/os/cmd_line.h>

#include <engine/application/application.h>
#include <engine/config/cvar.h>
#include <engine/machine/machine.h>
#include "engine/resource/types.h"
#include <engine/luasys/luasys.h>
#include <engine/renderer/renderer.h>
#include <engine/develop/develop_system.h>
#include <celib/thread/thread.h>
#include <engine/plugin/plugin.h>
#include <engine/memory/memsys.h>
#include <engine/static_systems.h>
#include <engine/input/keyboard.h>
#include <engine/input/mouse.h>
#include <engine/input/gamepad.h>

#include "engine/develop/console_server.h"
#include "engine/task/task.h"

#define LOG_WHERE "application"


IMPORT_API(ConsoleServerApi, 0);
IMPORT_API(DevelopSystemApi, 0);
IMPORT_API(RendererApi, 0);
IMPORT_API(ResourceApi, 0);
IMPORT_API(PackageApi, 0);
IMPORT_API(TaskApi, 0);
IMPORT_API(LuaSysApi, 0);
IMPORT_API(ConfigApi, 0);

//==============================================================================
// Globals
//==============================================================================

#define _G ApplicationGlobals

static struct G {
    struct {
        cvar_t boot_pkg;
        cvar_t boot_script;
        cvar_t screen_x;
        cvar_t screen_y;
        cvar_t fullscreen;

        cvar_t daemon;
        cvar_t compile;
        cvar_t continue_;
        cvar_t wait;
        cvar_t wid;
    } config;

    const struct game_callbacks *game;
    cel_window_t main_window;
    struct args args;
    int is_running;
    int init_error;
    float dt;
} ApplicationGlobals = {0};


//==============================================================================
// Private
//==============================================================================

static char _get_worker_id() {
    return 0;
}

//==============================================================================
// Interface
//==============================================================================

const char *application_platform();

const char *application_native_platform();

cel_window_t application_get_main_window();

void application_quit() {
    _G.is_running = 0;
    _G.init_error = 0;
}

static int _cmd_wait(mpack_node_t args,
                     mpack_writer_t *writer) {
    return 0;
}

static struct ApplicationApiV0 api_v1 = {
        .quit = application_quit,
        .platform =  application_platform,
        .native_platform =  application_native_platform,
        .main_window =  application_get_main_window,
};

int application_init(int argc,
                     const char **argv) {
    _G = (struct G) {0};
    _G.args = (struct args) {.argc = argc, .argv = argv};

    log_init(_get_worker_id);
    log_register_handler(log_stdout_handler, NULL);
    logdb_init_db(".");

    log_debug(LOG_WHERE, "Init (global size: %lu)", sizeof(struct G));

    memsys_init(4 * 1024 * 1024);

    ADD_STATIC_PLUGIN(memsys);
    ADD_STATIC_PLUGIN(config);
    ADD_STATIC_PLUGIN(application);

    cvar_init();

    _init_static_plugins();

    plugin_load_dirs("./bin");
    plugin_call_init_cvar();

    ConsoleServerApiV0 = *((struct ConsoleServerApiV0 *) plugin_get_engine_api(CONSOLE_SERVER_API_ID, 0));
    DevelopSystemApiV0 = *((struct DevelopSystemApiV0 *) plugin_get_engine_api(DEVELOP_SERVER_API_ID, 0));
    RendererApiV0 = *((struct RendererApiV0 *) plugin_get_engine_api(RENDERER_API_ID, 0));
    ResourceApiV0 = *((struct ResourceApiV0 *) plugin_get_engine_api(RESOURCE_API_ID, 0));
    PackageApiV0 = *((struct PackageApiV0 *) plugin_get_engine_api(PACKAGE_API_ID, 0));
    TaskApiV0 = *((struct TaskApiV0 *) plugin_get_engine_api(TASK_API_ID, 0));
    LuaSysApiV0 = *((struct LuaSysApiV0 *) plugin_get_engine_api(LUA_API_ID, 0));
    ConfigApiV0 = *((struct ConfigApiV0 *) plugin_get_engine_api(CONFIG_API_ID, 0));

    _G.config.boot_pkg = ConfigApiV0.new_str("core.boot_pkg", "Boot package", "boot");
    _G.config.boot_script = ConfigApiV0.new_str("core.boot_script", "Boot script", "lua/boot");

    _G.config.screen_x = ConfigApiV0.new_int("screen.x", "Screen width", 1024);
    _G.config.screen_y = ConfigApiV0.new_int("screen.y", "Screen height", 768);
    _G.config.fullscreen = ConfigApiV0.new_int("screen.fullscreen", "Fullscreen", 0);

    _G.config.daemon = ConfigApiV0.new_int("daemon", "Daemon mode", 0);
    _G.config.compile = ConfigApiV0.new_int("compile", "Comple", 0);
    _G.config.continue_ = ConfigApiV0.new_int("continue", "Continue after compile", 0);
    _G.config.wait = ConfigApiV0.new_int("wait", "Wait for client", 0);
    _G.config.wid = ConfigApiV0.new_int("wid", "Wid", 0);


    // Cvar stage

    ConfigApiV0.parse_core_args(_G.args);
    if (ConfigApiV0.get_int(_G.config.compile)) {
        ResourceApiV0.compiler_create_build_dir(ConfigApiV0, api_v1);
        ConfigApiV0.compile_global();
    }

    ConfigApiV0.load_global();

    if (!ConfigApiV0.parse_args(_G.args)) {
        return 0;
    }

    ConfigApiV0.log_all();

    plugin_call_init();

    log_set_wid_clb(TaskApiV0.worker_id);

    ConsoleServerApiV0.consolesrv_register_command("wait", _cmd_wait);

    return 1;
}

int application_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    plugin_call_shutdown();

    cvar_shutdown();
    memsys_shutdown();
    log_shutdown();

    return !_G.init_error;
}

static void _boot_stage() {
    stringid64_t boot_pkg = stringid64_from_string(ConfigApiV0.get_string(_G.config.boot_pkg));
    stringid64_t pkg = stringid64_from_string("package");

    stringid64_t core_pkg = stringid64_from_string("core");
    stringid64_t resources[] = {core_pkg, boot_pkg};

    ResourceApiV0.load_now(pkg, resources, 2);

    PackageApiV0.load(core_pkg);
    PackageApiV0.flush(core_pkg);
    PackageApiV0.load(boot_pkg);
    PackageApiV0.flush(boot_pkg);


    stringid64_t boot_script = stringid64_from_string(ConfigApiV0.get_string(_G.config.boot_script));
    LuaSysApiV0.execute_boot_script(boot_script);
}


static void _boot_unload() {
    stringid64_t boot_pkg = stringid64_from_string(ConfigApiV0.get_string(_G.config.boot_pkg));
    stringid64_t core_pkg = stringid64_from_string("core");
    stringid64_t pkg = stringid64_from_string("package");

    stringid64_t resources[] = {core_pkg, boot_pkg};

    PackageApiV0.unload(boot_pkg);
    PackageApiV0.unload(core_pkg);

    ResourceApiV0.unload(pkg, resources, 2);
}

void application_start() {
#if defined(CETECH_DEVELOP)
    ResourceApiV0.set_autoload(1);
#else
    ResourceApiV0.set_autoload(0);
#endif

    if (ConfigApiV0.get_int(_G.config.compile)) {
        ResourceApiV0.compiler_compile_all();

        if (!ConfigApiV0.get_int(_G.config.continue_)) {
            return;
        }
    }

    if (!ConfigApiV0.get_int(_G.config.daemon)) {
        intptr_t wid = ConfigApiV0.get_int(_G.config.wid);

        char title[128] = {0};
        snprintf(title, CEL_ARRAY_LEN(title), "cetech - %s", ConfigApiV0.get_string(_G.config.boot_script));

        if (wid == 0) {
            _G.main_window = cel_window_new(
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    ConfigApiV0.get_int(_G.config.screen_x), ConfigApiV0.get_int(_G.config.screen_y),
                    ConfigApiV0.get_int(_G.config.fullscreen) ? WINDOW_FULLSCREEN : WINDOW_NOFLAG
            );
        } else {
            _G.main_window = cel_window_new_from((void *) wid);
        }

        RendererApiV0.create(_G.main_window);
    }

    _boot_stage();

    u64 last_tick = cel_get_perf_counter();
    _G.game = LuaSysApiV0.get_game_callbacks();

    if (!_G.game->init()) {
        log_error(LOG_WHERE, "Could not init game.");
        return;
    };

    _G.is_running = 1;
    log_info("core.ready", "Run main loop");

    float lag = 0.0f;
    float frame_limit = 60.0f;
    float frame_time = (1.0f / frame_limit);
    float frame_time_accum = 0.0f;

    ConsoleServerApiV0.consolesrv_push_begin();
    while (_G.is_running) {
        struct scope_data application_sd = DevelopSystemApiV0.enter_scope("Application:update()");

        u64 now_ticks = cel_get_perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / cel_get_perf_freq();

        _G.dt = dt;
        last_tick = now_ticks;
        frame_time_accum += dt;

        plugin_call_update();
        _G.game->update(dt);

        if (frame_time_accum >= frame_time) {
            if (!ConfigApiV0.get_int(_G.config.daemon)) {
                struct scope_data render_sd = DevelopSystemApiV0.enter_scope("Game:render()");
                _G.game->render();
                DevelopSystemApiV0.leave_scope(render_sd);
            }

            frame_time_accum = 0.0f;
        }

        DevelopSystemApiV0.leave_scope(application_sd);
        DevelopSystemApiV0.push_record_float("engine.delta_time", dt);

        plugin_call_after_update(dt);
        //cel_thread_yield();
    }

    _G.game->shutdown();

    _boot_unload();
}


const char *application_native_platform() {
#if defined(CETECH_LINUX)
    return "linux";
#elif defined(CETECH_WINDOWS)
    return "windows";
#elif defined(CETECH_DARWIN)
    return "darwin";
#endif
    return NULL;
}

const char *application_platform() {
    return application_native_platform();
}

cel_window_t application_get_main_window() {
    return _G.main_window;
}


void *application_get_plugin_api(int api,
                                 int version) {

    if (api == PLUGIN_EXPORT_API_ID && version == 0) {
        static struct plugin_api_v0 plugin = {0};
        return &plugin;
    } else if (api == APPLICATION_API_ID && version == 0) {
        return &api_v1;
    }

    return 0;
}