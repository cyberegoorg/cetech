#include <stdio.h>

#include "cetech/application.h"
#include "cetech/config.h"
#include "cetech/module.h"
#include "cetech/input.h"

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

static void _init_api(get_api_fce_t get_engine_api) {
    keyboard_api_v0 = *((struct keyboard_api_v0 *) get_engine_api(KEYBOARD_API_ID));

//    log = get_engine_api(LOG_API_ID, 0);
//    mem = get_engine_api(MEMORY_API_ID, 0);
//    lua = get_engine_api(LUA_API_ID, 0);
}

static void _init(get_api_fce_t get_engine_api) {
    _init_api(get_engine_api);

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

static void *_reload_begin(get_api_fce_t get_engine_api) {
    //log->info("module_example", "Reload begin");

    return NULL;
}

static void _update() {
    if (keyboard_api_v0.button_state(0, keyboard_api_v0.button_index("v"))) {
        printf("dddddddddddddddddddddddddddddddddds 5  5 5 \n");
    }
}

static void _reload_end(get_api_fce_t get_engine_api,
                        void *data) {
    _init_api(get_engine_api);
    _init(get_engine_api);
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
