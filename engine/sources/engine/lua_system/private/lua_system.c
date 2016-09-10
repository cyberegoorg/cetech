//==============================================================================
// Includes
//==============================================================================

#include <include/mpack/mpack.h>
#include <celib/stringid/stringid.h>
#include <engine/resource_compiler/resource_compiler.h>
#include <celib/os/vio.h>
#include <celib/memory/memory.h>
#include <engine/resource_manager/resource_manager.h>
#include <engine/memory_system/memory_system.h>
#include <engine/application/application.h>
#include "celib/errors/errors.h"

#include "engine/lua_system/lua_system.h"
#include "engine/console_server/console_server.h"
#include "engine/plugin_system/plugin_system.h"

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "lua_system"

#define REGISTER_LUA_API(name) \
    void _register_lua_##name##_api();\
    _register_lua_##name##_api();


//==============================================================================
// Globals
//==============================================================================

struct lua_resource {
    u32 version;
    u32 size;
};

#define _G LuaGlobals

static struct G {
    lua_State *L;
    stringid64_t type_id;
} LuaGlobals = {0};


//==============================================================================
// Private
//==============================================================================

static int require(lua_State *L) {
    const char *name = lua_tostring(L, 1);
    stringid64_t name_hash = stringid64_from_string(name);

    struct lua_resource *resource = resource_get(_G.type_id, name_hash);

    if (resource == NULL) {
        return 0;
    }

    char *data = (char *) (resource + 1);

    luaL_loadbuffer(_G.L, data, resource->size, "<unknown>");

//    if (lua_pcall(_G.L, 0, 0, 0)) {
//        const char* last_error = lua_tostring(_G.L, -1);
//        lua_pop(_G.L, 1);
//        log_error(LOG_WHERE, "%s", last_error);
//    }

    return 1;
}

//==============================================================================
// Lua resource
//==============================================================================

void *lua_resource_loader(struct vio *input,
                          struct allocator *allocator) {
    const i64 size = vio_size(input);
    char *data = CE_ALLOCATE(allocator, char, size);
    vio_read(input, data, 1, size);

    return data;
}

void lua_resource_unloader(void *new_data,
                           struct allocator *allocator) {
    CE_DEALLOCATE(allocator, new_data);
}

void lua_resource_online(stringid64_t name,
                         void *data) {
}

void lua_resource_offline(stringid64_t name,
                          void *data) {

}

void *lua_resource_reloader(stringid64_t name,
                            void *old_data,
                            void *new_data,
                            struct allocator *allocator) {
    CE_DEALLOCATE(allocator, old_data);

    struct lua_resource *resource = new_data;
    char *data = (char *) (resource + 1);

    luaL_loadbuffer(_G.L, data, resource->size, "<unknown>");

    if (lua_pcall(_G.L, 0, 0, 0)) {
        const char *last_error = lua_tostring(_G.L, -1);
        lua_pop(_G.L, 1);
        log_error(LOG_WHERE, "%s", last_error);
    }

    return new_data;
}

static const resource_callbacks_t lua_resource_callback = {
        .loader = lua_resource_loader,
        .unloader =lua_resource_unloader,
        .online =lua_resource_online,
        .offline =lua_resource_offline,
        .reloader = lua_resource_reloader
};


//==============================================================================
// Game
//==============================================================================


int _game_init_clb() {
    luasys_call_global("init", NULL);
    return 1;
}

void _game_shutdown_clb() {
    luasys_call_global("shutdown", NULL);
}

void _game_update_clb(float dt) {
    luasys_call_global("update", "f", dt);
}

void _game_render_clb() {
    luasys_call_global("render", NULL);
}

static const struct game_callbacks _GameCallbacks = {
        .init = _game_init_clb,
        .shutdown = _game_shutdown_clb,
        .update = _game_update_clb,
        .render = _game_render_clb
};

static void _register_all_api() {
    REGISTER_LUA_API(log);
    REGISTER_LUA_API(keyboard);
    REGISTER_LUA_API(mouse);
    REGISTER_LUA_API(application);
    REGISTER_LUA_API(resource_compilator);
    REGISTER_LUA_API(resource_manager);
    REGISTER_LUA_API(renderer);
    REGISTER_LUA_API(world);
}

static int _reload_plugin(lua_State *l) {
    size_t len;
    const char *name = luasys_to_string_l(l, 1, &len);
    plugin_reload(name);
    return 0;
}

void _to_mpack(lua_State *_L,
               const int i,
               mpack_writer_t *writer) {
    int type = lua_type(_L, i);

    switch (type) {
        case LUA_TNUMBER: {
            uint32_t number = lua_tonumber(_L, i);
            mpack_write_i32(writer, number);
        }
            break;

        case LUA_TSTRING: {
            const char *str = lua_tostring(_L, i);
            mpack_write_cstr(writer, str);
        }
            break;

        case LUA_TBOOLEAN: {
            bool b = lua_toboolean(_L, i);
            mpack_write_bool(writer, b);
        }
            break;

        case LUA_TNIL: {
            mpack_write_nil(writer);
        }
            break;

        case LUA_TTABLE: {
            uint32_t count = 0;

            for (lua_pushnil(_L); lua_next(_L, -2); lua_pop(_L, 1)) {
                ++count;
            }

            mpack_start_map(writer, count);

            for (lua_pushnil(_L); lua_next(_L, -2); lua_pop(_L, 1)) {
                const char *key = lua_tostring(_L, -2);

                mpack_write_cstr(writer, key);
                _to_mpack(_L, lua_gettop(_L), writer);
            }

            mpack_finish_map(writer);
        }
            break;

        case LUA_TFUNCTION: {
            mpack_write_cstr(writer, "function");
        }
            break;

        default:
            return;
    }
}


static int _cmd_execute_string(mpack_node_t args,
                               mpack_writer_t *writer) {
    mpack_node_t node = mpack_node_map_cstr(args, "script");

    size_t str_len = mpack_node_strlen(node);
    const char *str = mpack_node_str(node);

    int top = lua_gettop(_G.L);

    if ((luaL_loadbuffer(_G.L, str, str_len, "console") || lua_pcall(_G.L, 0, LUA_MULTRET, 0))) {

        const char *last_error = lua_tostring(_G.L, -1);
        lua_pop(_G.L, 1);
        log_error(LOG_WHERE, "%s", last_error);

        mpack_start_map(writer, 1);
        mpack_write_cstr(writer, "error_msg");
        mpack_write_cstr(writer, last_error);
        mpack_finish_map(writer);

        return 1;
    }

    int nresults = lua_gettop(_G.L) - top;

    if (nresults != 0) {
        _to_mpack(_G.L, -1, writer);
        return 1;
    }

    return 0;
}

static int _execute_string(lua_State *_L,
                           const char *str) {
    if (luaL_dostring(_L, str)) {
        const char *last_error = lua_tostring(_L, -1);
        lua_pop(_L, 1);
        log_error(LOG_WHERE, "%s", last_error);
        return 0;
    }

    return 1;
}


//==============================================================================
// Resource compiler
//==============================================================================

int _lua_compiler(const char *filename,
                  struct vio *source_vio,
                  struct vio *build_vio,
                  struct compilator_api *compilator_api) {

    char tmp[vio_size(source_vio) + 1];
    memory_set(tmp, 0, vio_size(source_vio) + 1);

    vio_read(source_vio, tmp, sizeof(char), vio_size(source_vio));

    lua_State *state = luaL_newstate();
    luaL_openlibs(state);

    _execute_string(state,
                    "function compile(what, filename,  strip)\n"
                            " local s, err = loadstring(what, filename)\n"
                            " if s ~= nil then\n"
                            "   return string.dump(s, strip), nil\n"
                            " end\n"
                            " return nil, err\n"
                            "end"
    );

    lua_getglobal(state, "compile");
    luasys_push_string(state, tmp);
    luasys_push_string(state, filename);

#if defined(CETECH_DEBUG)
    luasys_push_bool(state, 0);
#else
    luasys_push_bool(state, 1);
#endif

    lua_pcall(state, 3, 2, 0);
    if (lua_isnil(state, 1)) {
        const char *err = luasys_to_string(state, 2);
        log_error("resource_compiler.lua", "[%s] %s", filename, err);

        lua_close(state);
        return 0;

    } else {
        size_t bc_len = 0;
        const char *bc = luasys_to_string_l(state, 1, &bc_len);

        struct lua_resource resource = {
                .version = 0,
                .size = bc_len,
        };

        vio_write(build_vio, &resource, sizeof(struct lua_resource), 1);
        vio_write(build_vio, bc, sizeof(char), bc_len);
    }

    lua_close(state);
    return 1;
}

//==============================================================================
// Interface
//==============================================================================

int luasys_num_args(lua_State *_L) {
    return lua_gettop(_L);
}

void luasys_remove(lua_State *_L,
                   int i) {
    lua_remove(_L, i);
}

void luasys_pop(lua_State *_L,
                int n) {
    lua_pop(_L, n);
}

int luasys_is_nil(lua_State *_L,
                  int i) {
    return lua_isnil(_L, i) == 1;
}

int luasys_is_number(lua_State *_L,
                     int i) {
    return lua_isnumber(_L, i) == 1;
}

int luasys_value_type(lua_State *_L,
                      int i) {
    return lua_type(_L, i);
}

void luasys_push_nil(lua_State *_L) {
    lua_pushnil(_L);
}

void luasys_push_int(lua_State *_L,
                     int value) {
    lua_pushinteger(_L, value);
}

void luasys_push_u64(lua_State *_L,
                     u64 value) {
    lua_pushinteger(_L, value);
}

void luasys_push_handler(lua_State *_L,
                         handler_t value) {
    lua_pushinteger(_L, value.h);
}

void luasys_push_bool(lua_State *_L,
                      int value) {
    lua_pushboolean(_L, value);
}

void luasys_push_float(lua_State *_L,
                       float value) {
    lua_pushnumber(_L, value);
}

void luasys_push_string(lua_State *_L,
                        const char *s) {
    lua_pushstring(_L, s);
}


int luasys_to_bool(lua_State *_L,
                   int i) {
    return lua_toboolean(_L, i);
}

int luasys_to_int(lua_State *_L,
                  int i) {
    return (int) lua_tointeger(_L, i);
}

float luasys_to_float(lua_State *_L,
                      int i) {
    return (float) lua_tonumber(_L, i);
}

handler_t luasys_to_handler(lua_State *l,
                            int i) {
    return (handler_t) {.h = lua_tonumber(l, i)};
}

const char *luasys_to_string(lua_State *_L,
                             int i) {
    return lua_tostring(_L, i);
}

const char *luasys_to_string_l(lua_State *_L,
                               int i,
                               size_t *len) {
    return lua_tolstring(_L, i, len);
}

int luasys_execute_string(const char *str) {
    return _execute_string(_G.L, str);
}

void luasys_execute_resource(stringid64_t name) {
    struct lua_resource *resource = resource_get(_G.type_id, name);
    char *data = (char *) (resource + 1);

    luaL_loadbuffer(_G.L, data, resource->size, "<unknown>");

    if (lua_pcall(_G.L, 0, 0, 0)) {
        const char *last_error = lua_tostring(_G.L, -1);
        lua_pop(_G.L, 1);
        log_error(LOG_WHERE, "%s", last_error);
    }
}

void luasys_add_module_function(const char *module,
                                const char *name,
                                const lua_CFunction func) {

    luaL_newmetatable(_G.L, "cetech");
    luaL_newmetatable(_G.L, module);

    luaL_Reg entry[2] = {
            {.name = name, .func = func},
            {0}
    };

    luaL_register(_G.L, NULL, entry);
    lua_setfield(_G.L, -2, module);
    lua_setglobal(_G.L, "cetech");
    lua_pop(_G.L, -1);
};


int luasys_init() {
    log_debug(LOG_WHERE, "Init");

    _G.L = luaL_newstate();
    CE_ASSERT(LOG_WHERE, _G.L != NULL);

    _G.type_id = stringid64_from_string("lua");

    luaL_openlibs(_G.L);

    lua_getfield(_G.L, LUA_GLOBALSINDEX, "package");
    lua_getfield(_G.L, -1, "loaders");
    lua_remove(_G.L, -2);

    int num_loaders = 0;
    lua_pushnil(_G.L);
    while (lua_next(_G.L, -2) != 0) {
        lua_pop(_G.L, 1);
        num_loaders++;
    }

    lua_pushinteger(_G.L, num_loaders + 1);
    lua_pushcfunction(_G.L, require);
    lua_rawset(_G.L, -3);
    lua_pop(_G.L, 1);

    _register_all_api();

    luasys_add_module_function("plugin", "reload", _reload_plugin);

    consolesrv_register_command("lua_system.execute", _cmd_execute_string);

    resource_register_type(stringid64_from_string("lua"), lua_resource_callback);
    resource_compiler_register(stringid64_from_string("lua"), _lua_compiler);

    return 1;
}

void luasys_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    lua_close(_G.L);
}

const struct game_callbacks *luasys_get_game_callbacks() {
    return &_GameCallbacks;
}

void luasys_execute_boot_script(stringid64_t name) {
    luasys_execute_resource(name);
}

void luasys_call_global(const char *func,
                        const char *args,
                        ...) {
    lua_State *_state = _G.L;

    uint32_t argc = 0;

    //lua_pushcfunction(L, error_handler);
    lua_getglobal(_state, func);

    if (args != NULL) {
        va_list vl;
        va_start(vl, args);

        const char *it = args;
        while (*it != '\0') {
            switch (*it) {
                case 'i':
                    luasys_push_int(_state, va_arg(vl, i32));
                    break;

//                case 'u':
//                    stack.push_uint32(va_arg(vl, uint32_t));
//                    break;

                case 'f':
                    luasys_push_float(_state, va_arg(vl, double));
                    break;
            }

            ++argc;
            ++it;
        }

        va_end(vl);
    }

    if (lua_pcall(_G.L, argc, 0, 0)) {
        const char *last_error = lua_tostring(_G.L, -1);
        lua_pop(_G.L, 1);
        log_error(LOG_WHERE, "%s", last_error);
    }

    lua_pop(_state, -1);
}