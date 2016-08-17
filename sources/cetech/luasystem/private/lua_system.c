#include <include/mpack/mpack.h>
#include <celib/errors/errors.h>

#include "cetech/luasystem/luasystem.h"

#define MAX_PLUGIN_NAME_LEN 64
#define LOG_WHERE "lua_system"

#include "include/luajit/lua.h"
#include "include/luajit/lauxlib.h"
#include "include/luajit/lualib.h"
#include "cetech/consoleserver/consoleserver.h"
#include "../../pluginsystem/plugin_system.h"

static struct {
    lua_State *L;
} _G;

static struct log_api_v0 *log = 0;

//////////////////////
int _num_args(lua_State *_L) {
    return lua_gettop(_L);
}

void _remove(lua_State *_L, int i) {
    lua_remove(_L, i);
}

void _pop(lua_State *_L, int n) {
    lua_pop(_L, n);
}

int _is_nil(lua_State *_L, int i) {
    return lua_isnil(_L, i) == 1;
}

int _is_number(lua_State *_L, int i) {
    return lua_isnumber(_L, i) == 1;
}

int _value_type(lua_State *_L, int i) {
    return lua_type(_L, i);
}

void _push_nil(lua_State *_L) {
    lua_pushnil(_L);
}

void _push_int(lua_State *_L, int value) {
    lua_pushinteger(_L, value);
}

void _push_bool(lua_State *_L, int value) {
    lua_pushboolean(_L, value);
}

void _push_float(lua_State *_L, float value) {
    lua_pushnumber(_L, value);
}

void _push_string(lua_State *_L, const char *s) {
    lua_pushstring(_L, s);
}


int _to_bool(lua_State *_L, int i) {
    return lua_tointeger(_L, i) == 1;
}

int _to_int(lua_State *_L, int i) {
    return (int) lua_tointeger(_L, i);
}

float _to_float(lua_State *_L, int i) {
    return (float) lua_tonumber(_L, i);
}

const char *_to_string(lua_State *_L, int i) {
    return lua_tostring(_L, i);
}

const char *_to_string_l(lua_State *_L, int i,
                         size_t *len) {
    return lua_tolstring(_L, i, len);
}

void to_mpack(lua_State *_L, const int i, mpack_writer_t *writer) {
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
                to_mpack(_L, lua_gettop(_L), writer);
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

int _execute_string(const char *str) {
    if (luaL_dostring(_G.L, str)) {
        const char *last_error = lua_tostring(_G.L, -1);
        lua_pop(_G.L, 1);
        log_error(LOG_WHERE, "%s", last_error);
        return 0;
    }

    return 1;
}

///////

//////////////////////

static int _reload_plugin(lua_State *l) {
    size_t len;
    const char *name = _to_string_l(l, 1, &len);
    plugin_reload(name);
    return 0;
}
///////


//////
static void _add_module_function(const char *module,
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

static int _cmd_execute_string(mpack_node_t args, mpack_writer_t *writer) {
    mpack_node_t node = mpack_node_map_cstr(args, "script");

    size_t str_len = mpack_node_strlen(node);
    const char *str = mpack_node_str(node);

    int top = lua_gettop(_G.L);

    if ((luaL_loadbuffer(_G.L, str, str_len, "console") ||
         lua_pcall(_G.L, 0, LUA_MULTRET, 0))) {

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
        to_mpack(_G.L, -1, writer);
        return 1;
    }

    return 0;
}


#define REGISTER_LUA_API(name) \
    void _register_lua_##name##_api();\
    _register_lua_##name##_api();


static void _register_all_api() {
    REGISTER_LUA_API(log);
    REGISTER_LUA_API(keyboard);
    REGISTER_LUA_API(mouse);
    REGISTER_LUA_API(application);

}

static void _init() {
    log_info(LOG_WHERE, "Init");

    _G.L = luaL_newstate();
    CE_ASSERT(LOG_WHERE, _G.L != NULL);

    luaL_openlibs(_G.L);

    _register_all_api();

    _add_module_function("plugin", "reload", _reload_plugin);

    consolesrv_register_command("luasystem.execute", _cmd_execute_string);
}

static void _shutdown() {
    log_info(LOG_WHERE, "Shutdown");

    lua_close(_G.L);
}


struct lua_api_v0 LUA = {
        .init = _init,
        .shutdown = _shutdown,
        .get_top = _num_args,
        .remove = _remove,
        .pop = _pop,
        .is_nil = _is_nil,
        .is_number = _is_number,
        .value_type = _value_type,
        .push_nil = _push_nil,
        .push_bool = _push_bool,
        .push_float = _push_float,
        .push_string = _push_string,
        .to_bool = _to_bool,
        .to_int = _to_int,
        .to_float = _to_float,
        .to_string = _to_string,
        .to_string_l = _to_string_l,
        .execute_string = _execute_string,
        .add_module_function = _add_module_function
};
