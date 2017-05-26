#include <stdio.h>
#include <cetech/kernel/api.h>

#include "cetech/kernel/application.h"
#include "cetech/kernel/config.h"
#include "cetech/kernel/module.h"
#include "cetech/kernel/input.h"

IMPORT_API(keyboard_api_v0)

// #includestatic struct KeyboardApiV0 KeyboardApiV0 = {0};

//
//static struct log_api_v0 *log = 0;
//static struct memory_api_v0 *mem = 0;
//static struct lua_api_v0 *lua = 0;
//static Alloc_t alloc = 0;
//
//
//static int _foo(lua_State *l) {
//    log->warning("fooo", "dasdsadsadas");
//    return 0;
//}
//
//static int _foo2(lua_State *l) {
//    log->error("fooo", "dasdsadsadas");
//    return 0;
//}
//

static void _init_api(struct api_v0* api) {
    GET_API(api, keyboard_api_v0);

//    log = get_engine_api(LOG_API_ID, 0);
//    mem = get_engine_api(MEMORY_API_ID, 0);
//    lua = get_engine_api(LUA_API_ID, 0);
}

static void _init( struct api_v0* api) {
    _init_api(api);

    printf("FOOOOO: %d", 11561651);
//
//    alloc = mem->create_module_allocator("example");
//
//    lua->add_module_function("example", "foo", _foo);
//    lua->add_module_function("example", "foo2", _foo2);
//
//    lua->execute_string("engine.example.foo()");
}

static void _shutdown() {
    //mem->destroy_module_allocator(alloc);
}

static void *_reload_begin( struct api_v0* api_v0) {
    //log->info("module_example", "Reload begin");

    return NULL;
}

static void _update() {
    if (keyboard_api_v0.button_state(0, keyboard_api_v0.button_index("v"))) {
        printf("dddddddddddddddddddddddddddddddddds 5  5 5 \n");
    }
}

static void _reload_end( struct api_v0* api,
                        void *data) {
    _init_api(api);
    _init(api);
}

void *get_module_api(int api,
                     int version) {

    if (api == PLUGIN_EXPORT_API_ID && version == 0) {
        static struct module_api_v0 module = {0};

        module.init = _init;
        module.shutdown = _shutdown;
        module.reload_begin = _reload_begin;
        module.reload_end = _reload_end;
        module.update = _update;

        return &module;
    }

    return 0;
}
