#ifndef CETECH_PRIVATE_LUASYS_H
#define CETECH_PRIVATE_LUASYS_H

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

int luasys_get_top(lua_State *l);

void luasys_remove(lua_State *l,
                   int idx);

void luasys_pop(lua_State *l,
                int idx);

int luasys_is_nil(lua_State *l,
                  int idx);

int luasys_is_number(lua_State *l,
                     int idx);

int luasys_value_type(lua_State *l,
                      int idx);

void luasys_push_nil(lua_State *l);

void luasys_push_u64(lua_State *l,
                     u64 value);

void luasys_push_handler(lua_State *l,
                         handler_t value);

void luasys_push_int(lua_State *l,
                     int value);

void luasys_push_bool(lua_State *l,
                      int value);

void luasys_push_float(lua_State *l,
                       float value);

void luasys_push_string(lua_State *l,
                        const char *value);

int luasys_to_bool(lua_State *l,
                   int i);

int luasys_to_int(lua_State *l,
                  int i);

f32 luasys_to_f32(lua_State *l,
                  int i);

handler_t luasys_to_handler(lua_State *l,
                            int i);

const char *luasys_to_string(lua_State *,
                             int i);

const char *luasys_to_string_l(lua_State *,
                               int,
                               size_t *);

cel_vec2f_t *luasys_to_vec2f(lua_State *l,
                             int i);

cel_vec3f_t *luasys_to_vec3f(lua_State *l,
                             int i);

cel_vec4f_t *luasys_to_vec4f(lua_State *l,
                             int i);

cel_mat44f_t *luasys_to_mat44f(lua_State *l,
                               int i);

cel_quatf_t *luasys_to_quat(lua_State *l,
                            int i);

void luasys_push_vec2f(lua_State *l,
                       cel_vec2f_t v);

void luasys_push_vec3f(lua_State *l,
                       cel_vec3f_t v);

void luasys_push_vec4f(lua_State *l,
                       cel_vec4f_t v);

void luasys_push_mat44f(lua_State *l,
                        cel_mat44f_t v);


void luasys_push_quat(lua_State *l,
                      cel_quatf_t v);

int luasys_execute_string(const char *str);

void luasys_add_module_function(const char *module,
                                const char *name,
                                const lua_CFunction func);

void luasys_add_module_constructor(const char *module,
                                   const lua_CFunction func);

void luasys_execute_resource(stringid64_t name);

const struct game_callbacks *luasys_get_game_callbacks();

void luasys_execute_boot_script(stringid64_t name);

void luasys_call_global(const char *func,
                        const char *args,
                        ...);


#endif //CETECH_PRIVATE_LUASYS_H
