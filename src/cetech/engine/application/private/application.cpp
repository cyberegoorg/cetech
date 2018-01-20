//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>

#include <cetech/core/macros.h>
#include <cetech/core/container_types.inl>

#include <cetech/core/map.inl>

#include <cetech/engine/application/application.h>
#include <cetech/core/watchdog.h>
#include <cetech/engine/filesystem/filesystem.h>
#include <cetech/core/private/ydb.h>
#include <cetech/core/api_system.h>
#include <cetech/engine/resource/package.h>
#include <cetech/core/task.h>
#include <cetech/engine/config/config.h>
#include <cetech/core/time.h>
#include <cetech/core/path.h>
#include <cetech/core/log.h>
#include <cetech/core/hashlib.h>
#include <cetech/core/memory.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/core/module.h>
#include <cetech/engine/kernel/kernel.h>

// TODO: shit , app == module?
#include <cetech/engine/machine/machine.h>
#include <cetech/engine/input/input.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/coredb/coredb.h>

#include <cetech/core/array.h>
#include <cetech/core/hash.h>
#include <cetech/core/buffer.h>

CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_package_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_time_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_watchdog_a0);
CETECH_DECL_API(ct_mouse_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_renderer_a0);
CETECH_DECL_API(ct_machine_a0);
CETECH_DECL_API(ct_coredb_a0);

//==============================================================================
// Definess
//==============================================================================

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

static struct ApplicationGlobals {
    ct_coredb_object_t *config_object;

    ct_app_on_init *on_init;
    ct_app_on_shutdown *on_shutdown;
    ct_app_on_update *on_update;

    ct_hash_t game_map;
    ct_game_fce *game;

    ct_game_fce active_game;
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

    ct_coredb_writer_t *writer = ct_coredb_a0.write_begin(_G.config_object);

    if (!ct_coredb_a0.prop_exist(_G.config_object, CONFIG_BOOT_PKG)) {
        ct_coredb_a0.set_string(writer, CONFIG_BOOT_PKG, "boot");
    }

    if (!ct_coredb_a0.prop_exist(_G.config_object, CONFIG_GAME)) {
        ct_coredb_a0.set_string(writer, CONFIG_GAME, "playground");
    }

    if (!ct_coredb_a0.prop_exist(_G.config_object, CONFIG_DAEMON)) {
        ct_coredb_a0.set_uint32(writer, CONFIG_DAEMON, 0);
    }

    if (!ct_coredb_a0.prop_exist(_G.config_object, CONFIG_COMPILE)) {
        ct_coredb_a0.set_uint32(writer, CONFIG_COMPILE, 0);
    }

    if (!ct_coredb_a0.prop_exist(_G.config_object, CONFIG_CONTINUE)) {
        ct_coredb_a0.set_uint32(writer, CONFIG_CONTINUE, 0);
    }

    if (!ct_coredb_a0.prop_exist(_G.config_object, CONFIG_WAIT)) {
        ct_coredb_a0.set_uint32(writer, CONFIG_WAIT, 0);
    }

    ct_coredb_a0.write_commit(writer);
}

static void _boot_stage() {
    const char *boot_pkg_str = ct_coredb_a0.read_string(_G.config_object,
                                                        CONFIG_BOOT_PKG, "");
    uint64_t boot_pkg = CT_ID64_0(boot_pkg_str);
    uint64_t pkg = CT_ID64_0("package");

    uint64_t core_pkg = CT_ID64_0("core/core");
    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_resource_a0.load_now(pkg, resources, 2);

    ct_package_a0.load(boot_pkg);
    ct_package_a0.flush(boot_pkg);

    ct_package_a0.load(core_pkg);
    ct_package_a0.flush(core_pkg);
}

static void _boot_unload() {
    const char *boot_pkg_str = ct_coredb_a0.read_string(_G.config_object,
                                                        CONFIG_BOOT_PKG, "");
    uint64_t boot_pkg = CT_ID64_0(boot_pkg_str);

    uint64_t core_pkg = CT_ID64_0("core/core");
    uint64_t pkg = CT_ID64_0("package");

    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_package_a0.unload(boot_pkg);
    ct_package_a0.unload(core_pkg);

    ct_resource_a0.unload(pkg, resources, 2);
}

void set_active_game(uint64_t name) {
    uint64_t idx = ct_hash_lookup(&_G.game_map, name, UINT64_MAX);
    if (idx != UINT64_MAX) {
        _G.active_game = _G.game[idx];
    }
}

static void check_machine() {
    ct_event_header *event = ct_machine_a0.event_begin();

    while (event != ct_machine_a0.event_end()) {
        switch (event->type) {
            case EVENT_QUIT:
                application_quit();
                break;

            default:
                break;
        }

        event = ct_machine_a0.event_next(event);
    }
}

extern "C" void application_start() {
    _init_config();

    if (ct_coredb_a0.read_uint32(_G.config_object, CONFIG_COMPILE, 0)) {
        ct_resource_a0.compiler_compile_all();

        if (!ct_coredb_a0.read_uint32(_G.config_object, CONFIG_CONTINUE, 0)) {
            return;
        }
    }

    _boot_stage();

    uint64_t last_tick = ct_time_a0.perf_counter();

    for (uint32_t i = 0; i < ct_array_size(_G.on_init); ++i) {
        _G.on_init[i]();
    }

    set_active_game(CT_ID64_0(ct_coredb_a0.read_string(_G.config_object,
                                                       CONFIG_GAME, "")));

    if (_G.active_game.on_init) {
        _G.active_game.on_init();
    }

    ct_coredb_object_t *obj1 = ct_coredb_a0.create_object();
    ct_coredb_object_t *obj2 = ct_coredb_a0.create_object();
    ct_coredb_object_t *obj3 = ct_coredb_a0.create_object();

    float f1 = ct_coredb_a0.read_float(obj1, 1, 22.0f);

    ct_coredb_writer_t *writer = ct_coredb_a0.write_begin(obj1);
    ct_coredb_a0.set_float(writer, 1, 44.0f);
    ct_coredb_a0.set_float(writer, 2, 55.0f);
    ct_coredb_a0.set_float(writer, 3, 66.0f);
    ct_coredb_a0.write_commit(writer);

    f1 = ct_coredb_a0.read_float(obj1, 1, 22.0f);
    f1 = ct_coredb_a0.read_float(obj1, 2, 22.0f);
    f1 = ct_coredb_a0.read_float(obj1, 3, 22.0f);

    CT_UNUSED(obj1)
    CT_UNUSED(f1)
    CT_UNUSED(obj2)
    CT_UNUSED(obj3)

    uint32_t *arr = NULL;
    ct_array_push(arr, 1, ct_memory_a0.main_allocator());
    ct_array_push(arr, 1, ct_memory_a0.main_allocator());
    ct_array_push(arr, 1, ct_memory_a0.main_allocator());

    uint32_t aaa[] = {1, 2};
    ct_array_push_n(arr, aaa, 2, ct_memory_a0.main_allocator());

    ct_array_pop_front(arr);
    ct_array_pop_front(arr);

    for (int j = 0; j < ct_array_size(arr); ++j) {
        ct_log_a0.debug("dddd", "%d", arr[j]);
    }

    ct_array_free(arr, ct_memory_a0.main_allocator());

    char* buffer = NULL;

    ct_buffer_printf(&buffer, _G.allocator, "foo %f", 10.f);
    ct_log_a0.info("dddsss", "%s", buffer);

//    ct_hash_t h1 = {};
//
//    ct_hash_add(&h1, CT_ID64_0("dva"), 2, _G.allocator);
//    ct_hash_add(&h1, CT_ID64_0("tri"), 3, _G.allocator);
//
//    for (int k = 1; k <100; ++k) {
//        ct_hash_add(&h1, k, 1, _G.allocator);
//
//    }
//
//    uint64_t jedna = ct_hash_lookup(&h1, CT_ID64_0("jedna"), 0);
//    uint64_t dva = ct_hash_lookup(&h1, CT_ID64_0("dva"), 0);
//    uint64_t tri = ct_hash_lookup(&h1, CT_ID64_0("tri"), 0);
//
//    CT_UNUSED(jedna)
//    CT_UNUSED(dva)
//    CT_UNUSED(tri)

    uint64_t fq = ct_time_a0.perf_freq();

//    ct_hash_t h1 = {};
//    uint64_t t1 = ct_time_a0.perf_counter();
//    uint64_t k;
//    for (k = 0; k < 100000; ++k) {
//        ct_hash_add(&h1, k, 0, _G.allocator);
//    }
//    uint64_t t2 = ct_time_a0.perf_counter();
//    float dt = ((float) (t2 - t1)) / fq;
//
//    t1 = ct_time_a0.perf_counter();
//    uint64_t idx = ct_hash_lookup(&h1, k-1, 0);
//    CT_UNUSED(dt, idx);
//    t2 = ct_time_a0.perf_counter();
//    dt = ((float) (t2 - t1)) / fq;
//
//    t1 = ct_time_a0.perf_counter();
//    core::Map<int> m2(_G.allocator);
//    for (k = 0; k < 100000; ++k) {
//        core::map::set(m2, k, 0);
//    }
//    CT_UNUSED(dt, idx);
//    t2 = ct_time_a0.perf_counter();
//    dt = ((float) (t2 - t1)) / fq;

    _G.is_running = 1;
    while (_G.is_running) {
        uint64_t now_ticks = ct_time_a0.perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        ct_filesystem_a0.check_wd();
        ct_ydb_a0.check_fs();
        ct_resource_a0.compiler_check_fs();

#if CETECH_DEVELOP
        ct_module_a0.check_modules(); // TODO: SHIT...
#endif
        ct_machine_a0.update(dt);
        check_machine();

        for (uint32_t i = 0; i < ct_array_size(_G.on_update); ++i) {
            _G.on_update[i](dt);
        }

        if (_G.active_game.on_update) {
            _G.active_game.on_update(dt);
        }

        CETECH_GET_API(&ct_api_a0, ct_mouse_a0); // TODO: WTF

        if (!ct_coredb_a0.read_uint32(_G.config_object, CONFIG_DAEMON, 0)) {
            ct_renderer_a0.render(
                    _G.active_game.on_render ? _G.active_game.on_render : NULL);
        }

        sleep(0);
    }

    if (_G.active_game.on_shutdown) {
        _G.active_game.on_shutdown();
    }

    for (uint32_t i = 0; i < ct_array_size(_G.on_shutdown); ++i) {
        _G.on_shutdown[i]();
    }

    _boot_unload();
}

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        ct_array_push(_G.name, name, _G.allocator);                           \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = ct_array_size(_G.name);                             \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            ct_array_pop_back(_G.name);                                       \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_app_on_init, on_init)

_DEF_ON_CLB_FCE(ct_app_on_shutdown, on_shutdown)

_DEF_ON_CLB_FCE(ct_app_on_update, on_update)


#undef _DEF_ON_CLB_FCE

void register_game(uint64_t name,
                   ct_game_fce game) {
    ct_array_push(_G.game, game, _G.allocator);
    ct_hash_add(&_G.game_map, name, ct_array_size(_G.game) - 1, _G.allocator);
}

void unregister_game(uint64_t name) {
    ct_hash_remove(&_G.game_map, name);
}


static ct_app_a0 a0 = {
        .quit = application_quit,
        .start = application_start,

        .register_game = register_game,
        .unregister_game = unregister_game,
        .set_active_game = set_active_game,

        .register_on_init = register_on_init_,
        .unregister_on_init = unregister_on_init_,

        .register_on_shutdown = register_on_shutdown_,
        .unregister_on_shutdown = unregister_on_shutdown_,

        .register_on_update = register_on_update_,
        .unregister_on_update = unregister_on_update_,

};

void app_init(struct ct_api_a0 *api) {
    api->register_api("ct_app_a0", &a0);

    _G.allocator = ct_memory_a0.main_allocator();

#if defined(CETECH_DEVELOP)
    ct_resource_a0.set_autoload(1);
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
            CETECH_GET_API(api, ct_hash_a0);

            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_package_a0);
            CETECH_GET_API(api, ct_task_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_module_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_ydb_a0);

            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_machine_a0);
            CETECH_GET_API(api, ct_coredb_a0);

            ct_api_a0 = *api;
        },
        {
            CT_UNUSED(reload);

            app_init(api);
        },
        {
            CT_UNUSED(api, reload);

            ct_array_free(_G.on_init, _G.allocator);
            ct_array_free(_G.on_shutdown, _G.allocator);
            ct_array_free(_G.on_update, _G.allocator);
            ct_array_free(_G.game, _G.allocator);
            ct_hash_free(&_G.game_map, _G.allocator);
        }
)
