//==============================================================================
// Includes
//==============================================================================

#include <unistd.h>


#include <cetech/kernel/application.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/package.h>
#include <cetech/kernel/task.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/time.h>
#include <cetech/kernel/path.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/memory.h>
#include <cetech/modules/machine/machine.h>
#include <cetech/kernel/resource.h>

#include <celib/macros.h>
#include <cetech/kernel/module.h>
#include <celib/container_types.inl>
#include <celib/array.inl>

#include <cetech/kernel/watchdog.h>
#include <cetech/modules/input/input.h>
#include <cetech/kernel/filesystem.h>
#include <cetech/kernel/ydb.h>

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
CETECH_DECL_API(ct_yamlng_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_ydb_a0);

//==============================================================================
// Definess
//==============================================================================

#define LOG_WHERE "application"

//==============================================================================
// Globals
//==============================================================================

struct GConfig {
    ct_cvar boot_pkg;
    ct_cvar screen_x;
    ct_cvar screen_y;
    ct_cvar fullscreen;

    ct_cvar daemon;
    ct_cvar compile;
    ct_cvar continue_;
    ct_cvar wait;
    ct_cvar wid;
};

static struct ApplicationGlobals {
    struct GConfig config;
    celib::Array<ct_app_on_init> on_init;
    celib::Array<ct_app_on_shutdown> on_shutdown;
    celib::Array<ct_app_on_update> on_update;
    celib::Array<ct_app_on_render> on_render;

    int is_running;
} _G;


//==============================================================================
// Interface
//==============================================================================

void application_quit() {
    _G.is_running = 0;
}

void _init_config() {
    _G.config = (struct GConfig) {
            .boot_pkg = ct_config_a0.new_str("core.boot_pkg", "Boot package",
                                             "boot"),

            .daemon = ct_config_a0.new_int("daemon", "Daemon mode", 0),
            .compile = ct_config_a0.new_int("compile", "Comple", 0),
            .continue_ = ct_config_a0.new_int("continue",
                                              "Continue after compile", 0),
            .wait = ct_config_a0.new_int("wait", "Wait for client", 0),
    };
}

static void _boot_stage() {
    const char *boot_pkg_str = ct_config_a0.get_string(_G.config.boot_pkg);
    uint64_t boot_pkg = ct_hash_a0.id64_from_str(boot_pkg_str);
    uint64_t pkg = ct_hash_a0.id64_from_str("package");

    uint64_t core_pkg = ct_hash_a0.id64_from_str("core");
    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_resource_a0.load_now(pkg, resources, 2);

    ct_package_a0.load(boot_pkg);
    ct_package_a0.flush(boot_pkg);

    ct_package_a0.load(core_pkg);
    ct_package_a0.flush(core_pkg);
}

static void _boot_unload() {
    const char *boot_pkg_str = ct_config_a0.get_string(_G.config.boot_pkg);
    uint64_t boot_pkg = ct_hash_a0.id64_from_str(boot_pkg_str);

    uint64_t core_pkg = ct_hash_a0.id64_from_str("core");
    uint64_t pkg = ct_hash_a0.id64_from_str("package");

    uint64_t resources[] = {core_pkg, boot_pkg};

    ct_package_a0.unload(boot_pkg);
    ct_package_a0.unload(core_pkg);

    ct_resource_a0.unload(pkg, resources, 2);
}


extern "C" void application_start() {
    ct_vio *f = ct_filesystem_a0.open(ct_hash_a0.id64_from_str("source"),
                                      "default.render_config",
                                      FS_OPEN_READ);

    ct_yamlng_document *d = ct_yamlng_a0.from_vio(f,
                                                  ct_memory_a0.main_allocator());
    ct_filesystem_a0.close(f);

    uint64_t key = ct_yamlng_a0.calc_key("entities.55643433135454252.prefab");
//    uint64_t key2 = ct_yamlng_a0.calc_key("entities.55643433135454252.prefab");

    uint64_t keys2[] = {
            ct_hash_a0.id64_from_str("viewport"),
            ct_hash_a0.id64_from_str("default"),
            ct_hash_a0.id64_from_str("local_resource"),
            ct_hash_a0.id64_from_str("2"),
            ct_hash_a0.id64_from_str("name"),
    };
    key = ct_yamlng_a0.combine_key(keys2, CETECH_ARRAY_LEN(keys2));

//    uint64_t keys[] = {
//            ct_hash_a0.id64_from_str("graph"),
//            //ct_hash_a0.id64_from_str("root"),
//    };
//    key = ct_yamlng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));

//    ct_yamlng_node node = d->get(d->inst, key);
//    ct_yamlng_node node2 = d->get(d->inst, key2);
//    CEL_UNUSED(node2);
//
//    d->foreach_dict_node(d->inst, node, [](
//                struct ct_yamlng_node key,
//                struct ct_yamlng_node value,
//                void *data) {
//                ct_yamlng_document* d = (ct_yamlng_document*)data;
//
//                uint64_t keys[] = {
//                        d->hash(d->inst, value),
//                        ct_hash_a0.id64_from_str("local"),
//                };
//                uint64_t  k = ct_yamlng_a0.combine_key(keys, CETECH_ARRAY_LEN(keys));
//
//                ct_yamlng_node node = d->get(d->inst, k);
//
//                //const char* s = d->as_string(d->inst, key, "NULL");
//                ct_log_a0.info(LOG_WHERE, "KEYS: %d", d->type(d->inst, node));
//        }, d);

//    ct_log_a0.info(LOG_WHERE, "MAAAAAAAAAA %s", d->get_string(d->inst, key, "NULL"));
//
//    {
//        uint64_t key = ct_yamlng_a0.calc_key("entities.55643433135454252.components.234234331453252");
//        ct_yamlng_node node = d->get(d->inst, key);
//        d->foreach_dict_node(d->inst, node, [](
//                struct ct_yamlng_node key,
//                struct ct_yamlng_node value,
//                void *data) {
//                ct_yamlng_document* d = (ct_yamlng_document*)data;
//
//                const char* s = d->as_string(d->inst, key, "NULL");
//
//                ct_log_a0.info(LOG_WHERE, "KEYS: %s : %d", s, d->type(d->inst, value));
//        }, d);
//    }
//
//    {
//        uint64_t key = ct_yamlng_a0.calc_key("entities.55643433135454252.components.234234331453252.position");
//        ct_yamlng_node node = d->get(d->inst, key);
//        d->foreach_seq_node(d->inst, node, [](
//                uint32_t idx,
//                struct ct_yamlng_node value,
//                void *data) {
//            ct_yamlng_document* d = (ct_yamlng_document*)data;
//
//            float f = d->as_float(d->inst, value, 0.0f);
//
//            ct_log_a0.info(LOG_WHERE, "VALUE : %f", f);
//        }, d);
//    }
//
//
//    {
//        uint64_t key = ct_yamlng_a0.calc_key("entities.55643433135454252.components.234234331453252.rotation");
//        ct_yamlng_node node = d->get(d->inst, key);
//
//        float v[3] {0.0f};
//        d->as_vec3(d->inst, node, v);
//
//        ct_log_a0.info(LOG_WHERE, "ROTATION : %f, %f, %f", v[0], v[1], v[2]);
//    }
//
    if (d->has_key(d->inst, key)) {
        ct_yamlng_node node = d->get(d->inst, key);
        ct_log_a0.info(LOG_WHERE, "MAAAAAAAAAA dsadsad %s: %d",
                       d->as_string(d->inst, node, ""), d->type(d->inst, node));
    }

    ct_yamlng_a0.destroy(d);

    uint64_t foo_k[] = {
            ct_yamlng_a0.calc_key("entities"),
            ct_yamlng_a0.calc_key("5564343313252"),
            ct_yamlng_a0.calc_key("components"),
            ct_yamlng_a0.calc_key("412321322234"),
            ct_yamlng_a0.calc_key("component_type"),
    };

//    uint64_t keys[32] = {};
//    uint32_t keys_count = 0;

    const char* type = ct_ydb_a0.get_string("level2.level", foo_k, CETECH_ARRAY_LEN(foo_k), "");
    CEL_UNUSED(type);

    //ct_ydb_a0.get_string("a.yaml", foo_k, CETECH_ARRAY_LEN(foo_k), "");

    //ct_log_a0.debug(LOG_WHERE, "FPOOOOOO: %s", foo);
    //return;

    _init_config();

    if (ct_config_a0.get_int(_G.config.compile)) {
        ct_resource_a0.compiler_compile_all();

        if (!ct_config_a0.get_int(_G.config.continue_)) {
            return;
        }
    }

    _boot_stage();

    uint64_t last_tick = ct_time_a0.perf_counter();

    for (uint32_t i = 0; i < celib::array::size(_G.on_init); ++i) {
        _G.on_init[i]();
    }

    _G.is_running = 1;
    while (_G.is_running) {
        uint64_t fq = ct_time_a0.perf_freq();
        uint64_t now_ticks = ct_time_a0.perf_counter();
        float dt = ((float) (now_ticks - last_tick)) / fq;
        last_tick = now_ticks;

        ct_filesystem_a0.check_wd();

#if CETECH_DEVELOP
        ct_module_a0.check_modules(); // TODO: SHIT...
#endif

        for (uint32_t i = 0; i < celib::array::size(_G.on_update); ++i) {
            _G.on_update[i](dt);
        }

        CETECH_GET_API(&ct_api_a0, ct_mouse_a0);

        if (!ct_config_a0.get_int(_G.config.daemon)) {
            for (uint32_t i = 0; i < celib::array::size(_G.on_render); ++i) {
                _G.on_render[i]();
            }
        }

        sleep(0);
    }

    for (uint32_t i = 0; i < celib::array::size(_G.on_shutdown); ++i) {
        _G.on_shutdown[i]();
    }

    _boot_unload();
}

#define _DEF_ON_CLB_FCE(type, name)                                            \
    static void register_ ## name ## _(type name) {                            \
        celib::array::push_back(_G.name, name);                                \
    }                                                                          \
    static void unregister_## name ## _(type name) {                           \
        const auto size = celib::array::size(_G.name);                         \
                                                                               \
        for(uint32_t i = 0; i < size; ++i) {                                   \
            if(_G.name[i] != name) {                                           \
                continue;                                                      \
            }                                                                  \
                                                                               \
            uint32_t last_idx = size - 1;                                      \
            _G.name[i] = _G.name[last_idx];                                    \
                                                                               \
            celib::array::pop_back(_G.name);                                   \
            break;                                                             \
        }                                                                      \
    }

_DEF_ON_CLB_FCE(ct_app_on_init, on_init)

_DEF_ON_CLB_FCE(ct_app_on_shutdown, on_shutdown)

_DEF_ON_CLB_FCE(ct_app_on_update, on_update)

_DEF_ON_CLB_FCE(ct_app_on_render, on_render)

#undef _DEF_ON_CLB_FCE

static ct_app_a0 a0 = {
        .quit = application_quit,
        .start = application_start,

        .register_on_init = register_on_init_,
        .unregister_on_init = unregister_on_init_,

        .register_on_shutdown = register_on_shutdown_,
        .unregister_on_shutdown = unregister_on_shutdown_,

        .register_on_update = register_on_update_,
        .unregister_on_update = unregister_on_update_,

        .register_on_render = register_on_render_,
        .unregister_on_render = unregister_on_render_
};

void app_init(struct ct_api_a0 *api) {
    api->register_api("ct_app_a0", &a0);

#if defined(CETECH_DEVELOP)
    ct_resource_a0.set_autoload(1);
#else
    ct_resource_a0.set_autoload(0);
#endif

    _G.on_update.init(ct_memory_a0.main_allocator());
    _G.on_init.init(ct_memory_a0.main_allocator());
    _G.on_shutdown.init(ct_memory_a0.main_allocator());
    _G.on_render.init(ct_memory_a0.main_allocator());
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
            CETECH_GET_API(api, ct_yamlng_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_ydb_a0);

            ct_api_a0 = *api;
        },
        {
            app_init(api);
        },
        {
            CEL_UNUSED(api);
            _G.on_init.destroy();
            _G.on_shutdown.destroy();
            _G.on_update.destroy();
            _G.on_render.destroy();
        }
)
