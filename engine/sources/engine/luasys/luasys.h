//! \defgroup Lua
//! Lua system
//! \{
#ifndef CETECH_LUA_TYPES_H
#define CETECH_LUA_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/luajit/lua.h"
#include "include/luajit/lauxlib.h"
#include "include/luajit/lualib.h"

#include "celib/math/types.h"
#include <celib/handler/handlerid.h>
#include <celib/string/stringid.h>

//==============================================================================
// Interface
//==============================================================================

struct LuaSysApiV1 {
    int (*get_top)(lua_State *l);

    void (*remove)(lua_State *l,
                   int idx);

    void (*pop)(lua_State *l,
                int idx);

    int (*is_nil)(lua_State *l,
                  int idx);

    int (*is_number)(lua_State *l,
                     int idx);

    int (*value_type)(lua_State *l,
                      int idx);

    void (*push_nil)(lua_State *l);

    void (*push_u64)(lua_State *l,
                     u64 value);

    void (*push_handler)(lua_State *l,
                         handler_t value);

    void (*push_int)(lua_State *l,
                     int value);

    void (*push_bool)(lua_State *l,
                      int value);

    void (*push_float)(lua_State *l,
                       float value);

    void (*push_string)(lua_State *l,
                        const char *value);

    int (*to_bool)(lua_State *l,
                   int i);

    int (*to_int)(lua_State *l,
                  int i);

    f32 (*to_f32)(lua_State *l,
                  int i);

    handler_t (*to_handler)(lua_State *l,
                            int i);

    const char *(*to_string)(lua_State *,
                             int i);

    const char *(*to_string_l)(lua_State *l,
                               int i,
                               size_t *len);

    cel_vec2f_t *(*to_vec2f)(lua_State *l,
                             int i);

    cel_vec3f_t *(*to_vec3f)(lua_State *l,
                             int i);

    cel_vec4f_t *(*to_vec4f)(lua_State *l,
                             int i);

    cel_mat44f_t *(*to_mat44f)(lua_State *l,
                               int i);

    cel_quatf_t *(*to_quat)(lua_State *l,
                            int i);

    void (*push_vec2f)(lua_State *l,
                       cel_vec2f_t v);

    void (*push_vec3f)(lua_State *l,
                       cel_vec3f_t v);

    void (*push_vec4f)(lua_State *l,
                       cel_vec4f_t v);

    void (*push_mat44f)(lua_State *l,
                        cel_mat44f_t v);

    void (*push_quat)(lua_State *l,
                      cel_quatf_t v);

    int (*execute_string)(const char *str);

    void (*add_module_function)(const char *module,
                                const char *name,
                                const lua_CFunction func);

    void (*add_module_constructor)(const char *module,
                                   const lua_CFunction func);

    void (*execute_resource)(stringid64_t name);

    const struct game_callbacks *(*get_game_callbacks)();

    void (*execute_boot_script)(stringid64_t name);

    void (*call_global)(const char *func,
                        const char *args,
                        ...);
};

#endif //CETECH_LUA_TYPES_H
//! \}