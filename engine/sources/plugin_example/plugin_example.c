#include <stdio.h>
#include <cetech/core/api.h>

#include "cetech/core/config.h"
#include "cetech/core/module.h"
#include "cetech/modules/input/input.h"

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

void _init_api(struct api_v0 *api) {
    GET_API(api, keyboard_api_v0);

//    log = get_engine_api(LOG_API_ID, 0);
//    mem = get_engine_api(MEMORY_API_ID, 0);
//    lua = get_engine_api(LUA_API_ID, 0);
}

void init(struct api_v0 *api) {
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

void _shutdown() {
    //mem->destroy_module_allocator(alloc);
}

void *_reload_begin(struct api_v0 *api_v0) {
    //log->info("module_example", "Reload begin");

    return NULL;
}

void _update() {
    if (keyboard_api_v0.button_state(0, keyboard_api_v0.button_index("v"))) {
        printf("dddddddddddddddddddddddddddddddddds\n");
    }
}

void _reload_end(struct api_v0 *api,
                 void *data) {
    _init_api(api);
    init(api);
}

void *get_module_api(int api) {

    if (api == PLUGIN_EXPORT_API_ID) {
        static struct module_api_v0 module = {0};

        module.init = init;
        module.shutdown = _shutdown;
        module.reload_begin = _reload_begin;
        module.reload_end = _reload_end;
        module.update = _update;

        return &module;
    }

    return 0;
}
