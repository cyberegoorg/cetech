//==============================================================================
// Includes
//==============================================================================

#include <include/mpack/mpack.h>
#include <celib/string/stringid.h>
#include <celib/filesystem/vio.h>
#include <engine/application/application.h>
#include <celib/math/types.h>
#include <celib/math/vec2f.h>

#include "engine/develop/console_server.h"
#include "engine/module/module.h"

#include "vectors.h"
#include "quaternion.h"
#include "matrix.h"
#include "engine/luasys/luasys.h"
#include "engine/resource/types.h"


IMPORT_API(ResourceApi, 0);
IMPORT_API(ConsoleServerApi, 0);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "lua_system"

#define TEMP_VAR_COUNT 1024


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

    u32 _temp_cel_vec2f_used;
    u32 _temp_cel_vec3f_used;
    u32 _temp_cel_vec4f_used;
    u32 _temp_cel_mat44f_used;
    u32 _temp_quat_used;

    cel_vec2f_t _temp_cel_vec2f_buffer[TEMP_VAR_COUNT];
    cel_vec3f_t _temp_cel_vec3f_buffer[TEMP_VAR_COUNT];
    cel_vec4f_t _temp_cel_vec4f_buffer[TEMP_VAR_COUNT];
    cel_mat44f_t _temp_cel_mat44f_buffer[TEMP_VAR_COUNT];
    cel_quatf_t _temp_quat_buffer[TEMP_VAR_COUNT];

} LuaGlobals = {0};

//==============================================================================
// Private
//==============================================================================

static int require(lua_State *L) {
    const char *name = lua_tostring(L, 1);
    stringid64_t name_hash = stringid64_from_string(name);

    struct lua_resource *resource = ResourceApiV0.get(_G.type_id, name_hash);

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

cel_vec2f_t *_new_tmp_vec2f() {
    CEL_ASSERT("lua_enviroment", _G._temp_cel_vec2f_used < 1024);
    return &_G._temp_cel_vec2f_buffer[_G._temp_cel_vec2f_used++];
}

cel_vec3f_t *_new_tmp_vec3f() {
    CEL_ASSERT("lua_enviroment", _G._temp_cel_vec3f_used < 1024);
    return &_G._temp_cel_vec3f_buffer[_G._temp_cel_vec3f_used++];
}

cel_vec4f_t *_new_tmp_vec4f() {
    CEL_ASSERT("lua_enviroment", _G._temp_cel_vec4f_used < 1024);
    return &_G._temp_cel_vec4f_buffer[_G._temp_cel_vec4f_used++];
}

cel_mat44f_t *_new_tmp_mat44f() {
    CEL_ASSERT("lua_enviroment", _G._temp_cel_mat44f_used < 1024);
    return &_G._temp_cel_mat44f_buffer[_G._temp_cel_mat44f_used++];
}

cel_quatf_t *_new_tmp_quat() {
    CEL_ASSERT("lua_enviroment", _G._temp_quat_used < 1024);
    return &_G._temp_quat_buffer[_G._temp_quat_used++];
}

//==============================================================================
// Lua resource
//==============================================================================

void *lua_resource_loader(struct vio *input,
                          struct cel_allocator *allocator) {
    const i64 size = cel_vio_size(input);
    char *data = CEL_ALLOCATE(allocator, char, size);
    cel_vio_read(input, data, 1, size);

    return data;
}

void lua_resource_unloader(void *new_data,
                           struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, new_data);
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
                            struct cel_allocator *allocator) {
    CEL_DEALLOCATE(allocator, old_data);

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
    _G._temp_cel_vec2f_used = 0;
    _G._temp_cel_vec3f_used = 0;
    _G._temp_cel_vec4f_used = 0;
    _G._temp_cel_mat44f_used = 0;
    _G._temp_quat_used = 0;

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

#define REGISTER_LUA_API(name) \
    void _register_lua_##name##_api();\
    _register_lua_##name##_api(get_engine_api);

static void _register_all_api(get_api_fce_t get_engine_api) {
    REGISTER_LUA_API(log);
    REGISTER_LUA_API(module);
    REGISTER_LUA_API(keyboard);
    REGISTER_LUA_API(mouse);
    REGISTER_LUA_API(gamepad);
    REGISTER_LUA_API(application);
    REGISTER_LUA_API(resource_manager);
    REGISTER_LUA_API(renderer);
    REGISTER_LUA_API(world);
    REGISTER_LUA_API(unit);
    REGISTER_LUA_API(transform);
    REGISTER_LUA_API(vec2f);
    REGISTER_LUA_API(vec3f);
    REGISTER_LUA_API(vec4f);
    REGISTER_LUA_API(mat44f);
    REGISTER_LUA_API(quatf);
    REGISTER_LUA_API(mesh);
    REGISTER_LUA_API(material);
    REGISTER_LUA_API(package);
    REGISTER_LUA_API(level);
    REGISTER_LUA_API(scenegraph);
}

static int _reload_module(lua_State *l) {
    size_t len;
    const char *name = luasys_to_string_l(l, 1, &len);
    module_reload(name);
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

    char tmp[cel_vio_size(source_vio) + 1];
    memory_set(tmp, 0, cel_vio_size(source_vio) + 1);

    cel_vio_read(source_vio, tmp, sizeof(char), cel_vio_size(source_vio));

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

        cel_vio_write(build_vio, &resource, sizeof(struct lua_resource), 1);
        cel_vio_write(build_vio, bc, sizeof(char), bc_len);
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

f32 luasys_to_f32(lua_State *_L,
                  int i) {
    return (f32) lua_tonumber(_L, i);
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

cel_vec2f_t *luasys_to_vec2f(lua_State *l,
                             int i) {
    void *v = lua_touserdata(l, i);
    return (cel_vec2f_t *) v;
}

cel_vec3f_t *luasys_to_vec3f(lua_State *l,
                             int i) {
    void *v = lua_touserdata(l, i);
    return (cel_vec3f_t *) v;
}

cel_vec4f_t *luasys_to_vec4f(lua_State *l,
                             int i) {
    void *v = lua_touserdata(l, i);
    return (cel_vec4f_t *) v;
}

cel_quatf_t *luasys_to_quat(lua_State *l,
                            int i) {
    void *v = lua_touserdata(l, i);
    return (cel_quatf_t *) v;
}

void luasys_push_vec2f(lua_State *l,
                       cel_vec2f_t v) {
    cel_vec2f_t *tmp_v = _new_tmp_vec2f();
    *tmp_v = v;

    lua_pushlightuserdata(l, tmp_v);
}

void luasys_push_vec3f(lua_State *l,
                       cel_vec3f_t v) {
    cel_vec3f_t *tmp_v = _new_tmp_vec3f();
    *tmp_v = v;

    lua_pushlightuserdata(l, tmp_v);
}

void luasys_push_vec4f(lua_State *l,
                       cel_vec4f_t v) {
    cel_vec4f_t *tmp_v = _new_tmp_vec4f();
    *tmp_v = v;

    lua_pushlightuserdata(l, tmp_v);
}

void luasys_push_quat(lua_State *l,
                      cel_quatf_t v) {
    cel_quatf_t *tmp_v = _new_tmp_quat();
    *tmp_v = v;

    lua_pushlightuserdata(l, tmp_v);
}

cel_mat44f_t *luasys_to_mat44f(lua_State *l,
                               int i) {
    void *v = lua_touserdata(l, i);
    return (cel_mat44f_t *) v;
}

void luasys_push_mat44f(lua_State *l,
                        cel_mat44f_t v) {
    cel_mat44f_t *tmp_v = _new_tmp_mat44f();
    *tmp_v = v;

    lua_pushlightuserdata(l, tmp_v);
}

int luasys_execute_string(const char *str) {
    return _execute_string(_G.L, str);
}

void luasys_execute_resource(stringid64_t name) {
    struct lua_resource *resource = ResourceApiV0.get(_G.type_id, name);
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

//void luasys_add_module_constructor(const char* module,
//                                   const lua_CFunction func) {
//
//    lua_createtable(_G.L, 0, 1);
//    lua_pushstring(_G.L, "__call");
//    lua_pushcfunction(_G.L, func);
//    lua_settable(_G.L, 1);
//    lua_getglobal(_G.L, "cetech");
//    lua_getglobal(_G.L, module);
//    lua_pushvalue(_G.L, -2);
//    lua_setmetatable(_G.L, -2);
//    lua_pop(_G.L, -1);
//}


static int _is_vec2f(lua_State *L,
                     cel_vec2f_t *p) {
    return (p >= _G._temp_cel_vec2f_buffer) &&
           (p < (_G._temp_cel_vec2f_buffer + 1024));
}

static int _is_vec3f(lua_State *L,
                     cel_vec3f_t *p) {
    return (p >= _G._temp_cel_vec3f_buffer) &&
           (p < (_G._temp_cel_vec3f_buffer + 1024));
}

static int _is_vec4f(lua_State *L,
                     cel_vec4f_t *p) {
    return (p >= _G._temp_cel_vec4f_buffer) &&
           (p < (_G._temp_cel_vec4f_buffer + 1024));
}

static int _is_quat(lua_State *L,
                    cel_quatf_t *p) {
    return (p >= _G._temp_quat_buffer) && (p < (_G._temp_quat_buffer + 1024));
}

static int _is_mat44f(lua_State *L,
                      cel_mat44f_t *p) {
    return (p >= _G._temp_cel_mat44f_buffer) &&
           (p < (_G._temp_cel_mat44f_buffer + 1024));
}


static int lightuserdata_add(lua_State *L) {
    void *p = lua_touserdata(L, 1);

    if (_is_vec2f(L, p)) {
        return _cel_vec2f_add(L);
    }

    if (_is_vec3f(L, p)) {
        return _cel_vec3f_add(L);
    }

    if (_is_vec4f(L, p)) {
        return _cel_vec4f_add(L);
    }

    if (_is_quat(L, p)) {
        return _quat_add(L);
    }

    return 0;
}

static int lightuserdata_sub(lua_State *L) {
    void *p = lua_touserdata(L, 1);

    if (_is_vec2f(L, p)) {
        return _cel_vec2f_sub(L);
    }

    if (_is_vec3f(L, p)) {
        return _cel_vec3f_sub(L);
    }

    if (_is_vec4f(L, p)) {
        return _cel_vec4f_sub(L);
    }

    if (_is_quat(L, p)) {
        return _quat_sub(L);
    }

//    if( _is_mat44f(L, p)) {
//        return _cel_mat44f_add(L);
//    }
    return 0;
}

static int lightuserdata_mul(lua_State *L) {
    void *p = lua_touserdata(L, 1);

    if (_is_vec2f(L, p)) {
        return _cel_vec2f_mul(L);
    }

    if (_is_vec3f(L, p)) {
        return _cel_vec3f_mul(L);
    }

    if (_is_vec4f(L, p)) {
        return _cel_vec4f_mul(L);
    }

    if (_is_quat(L, p)) {
        return _quat_mul(L);
    }

    if (_is_mat44f(L, p)) {
        return _cel_mat44f_mul(L);
    }
    return 0;
}

static int lightuserdata_div(lua_State *L) {
    void *p = lua_touserdata(L, 1);

    if (_is_vec2f(L, p)) {
        return _cel_vec2f_div(L);
    }

    if (_is_vec3f(L, p)) {
        return _cel_vec3f_div(L);
    }

    if (_is_vec4f(L, p)) {
        return _cel_vec4f_div(L);
    }

    if (_is_quat(L, p)) {
        return _quat_div(L);
    }

//    if( _is_mat44f(L, p)) {
//        return _cel_mat44f_add(L);
//    }}
    return 0;
}

static int lightuserdata_unm(lua_State *L) {
    void *p = lua_touserdata(L, 1);

    if (_is_vec2f(L, p)) {
        return _cel_vec2f_unm(L);
    }

    if (_is_vec3f(L, p)) {
        return _cel_vec3f_unm(L);
    }

    if (_is_vec4f(L, p)) {
        return _cel_vec4f_unm(L);
    }

    if (_is_quat(L, p)) {
        return _quat_unm(L);
    }

    if (_is_mat44f(L, p)) {
        return _cel_mat44f_unm(L);
    }

    return 0;
}

static int lightuserdata_index(lua_State *L) {
    void *p = lua_touserdata(L, 1);

    if (_is_vec2f(L, p)) {
        return _cel_vec2f_index(L);
    }

    if (_is_vec3f(L, p)) {
        return _cel_vec3f_index(L);
    }

    if (_is_vec4f(L, p)) {
        return _cel_vec4f_index(L);
    }

    if (_is_quat(L, p)) {
        return _quat_index(L);
    }

    if (_is_mat44f(L, p)) {
        return _cel_mat44f_index(L);
    }

    return 0;
}

static int lightuserdata_newindex(lua_State *L) {
    void *p = lua_touserdata(L, 1);

    if (_is_vec2f(L, p)) {
        return _cel_vec2f_newindex(L);
    }

    if (_is_vec3f(L, p)) {
        return _cel_vec3f_newindex(L);
    }

    if (_is_vec4f(L, p)) {
        return _cel_vec4f_newindex(L);
    }

    if (_is_quat(L, p)) {
        return _quat_newindex(L);
    }

    if (_is_mat44f(L, p)) {
        return _cel_mat44f_newindex(L);
    }

    return 0;
}

void _create_lightuserdata() {
    luasys_add_module_function("lightuserdata_mt", "__add", lightuserdata_add);
    luasys_add_module_function("lightuserdata_mt", "__sub", lightuserdata_sub);
    luasys_add_module_function("lightuserdata_mt", "__mul", lightuserdata_mul);
    luasys_add_module_function("lightuserdata_mt", "__unm", lightuserdata_unm);
    luasys_add_module_function("lightuserdata_mt", "__index",
                               lightuserdata_index);
    luasys_add_module_function("lightuserdata_mt", "__newindex",
                               lightuserdata_newindex);

    lua_pushlightuserdata(_G.L, 0);
    lua_getfield(_G.L, LUA_REGISTRYINDEX, "lightuserdata_mt");
    lua_setmetatable(_G.L, -2);
    lua_pop(_G.L, 1);
}


static void _init(get_api_fce_t get_engine_api) {
    log_debug(LOG_WHERE, "Init");

    INIT_API(ConsoleServerApi, CONSOLE_SERVER_API_ID, 0);
    INIT_API(ResourceApi, RESOURCE_API_ID, 0);

    _G.L = luaL_newstate();
    CEL_ASSERT(LOG_WHERE, _G.L != NULL);

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

    _create_lightuserdata();

    _register_all_api(get_engine_api);

    luasys_add_module_function("module", "reload", _reload_module);
    ConsoleServerApiV0.consolesrv_register_command("lua_system.execute",
                                                   _cmd_execute_string);

    ResourceApiV0.register_type(_G.type_id, lua_resource_callback);
    ResourceApiV0.compiler_register(_G.type_id, _lua_compiler);
}

static void _shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    lua_close(_G.L);

    _G = (struct G) {0};
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

void *luasys_get_module_api(int api,
                            int version) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

                default:
                    return NULL;
            };
        case LUA_API_ID:
            switch (version) {
                case 0: {
                    static struct LuaSysApiV0 api = {0};

                    //api.get_top = luasys_get_top;
                    api.remove = luasys_remove;
                    api.pop = luasys_pop;
                    api.is_nil = luasys_is_nil;
                    api.is_number = luasys_is_number;
                    api.value_type = luasys_value_type;
                    api.push_nil = luasys_push_nil;
                    api.push_u64 = luasys_push_u64;
                    api.push_handler = luasys_push_handler;
                    api.push_int = luasys_push_int;
                    api.push_bool = luasys_push_bool;
                    api.push_float = luasys_push_float;
                    api.push_string = luasys_push_string;
                    api.to_bool = luasys_to_bool;
                    api.to_int = luasys_to_int;
                    api.to_f32 = luasys_to_f32;
                    api.to_handler = luasys_to_handler;
                    api.to_string = luasys_to_string;
                    api.to_string_l = luasys_to_string_l;
                    api.to_vec2f = luasys_to_vec2f;
                    api.to_vec3f = luasys_to_vec3f;
                    api.to_vec4f = luasys_to_vec4f;
                    api.to_mat44f = luasys_to_mat44f;
                    api.to_quat = luasys_to_quat;
                    api.push_vec2f = luasys_push_vec2f;
                    api.push_vec3f = luasys_push_vec3f;
                    api.push_vec4f = luasys_push_vec4f;
                    api.push_mat44f = luasys_push_mat44f;
                    api.push_quat = luasys_push_quat;
                    api.execute_string = luasys_execute_string;
                    api.add_module_function = luasys_add_module_function;
                    //api.add_module_constructor = luasys_add_module_constructor;
                    api.execute_resource = luasys_execute_resource;
                    api.get_game_callbacks = luasys_get_game_callbacks;
                    api.execute_boot_script = luasys_execute_boot_script;
                    api.call_global = luasys_call_global;

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }
}