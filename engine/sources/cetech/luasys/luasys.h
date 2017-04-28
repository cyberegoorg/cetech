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

#include "celib/math_types.h"
#include <celib/handlerid.h>
#include <celib/stringid.h>

//==============================================================================
// Api
//==============================================================================

//! Lua system api V0
struct LuaSysApiV0 {

    //! Return number of elements in the stack, which is also the index of the top element.
    //! Notice that a negative index -x is equivalent to the positive index gettop - x + 1.
    //! \param l Lua state
    //! \return Number of elements
    int (*get_top)(lua_State *l);

    //! Removes the element at the given index, shifting down all elements on top of that position to fill in the gap
    //! \param l Lua state
    //! \param idx Elenebt idx
    void (*remove)(lua_State *l,
                   int idx);

    //! Remove n elements from stack
    //! \param l Lua state
    //! \param n Numbers of elements to remove
    void (*pop)(lua_State *l,
                int idx);

    //! Is value nil?
    //! \param l Lua state
    //! \param idx Element idx
    //! \return 1 if is nil else 0
    int (*is_nil)(lua_State *l,
                  int idx);

    //! Is value number?
    //! \param l Lua state
    //! \param idx Element idx
    //! \return 1 if is numberelse 0
    int (*is_number)(lua_State *l,
                     int idx);

    int (*is_vec2f)(lua_State *L,
                        int idx);

    int (*is_vec3f)(lua_State *L,
                        int idx);

    int (*is_vec4f)(lua_State *L,
                        int idx);

    int (*is_quat)(lua_State *L,
                       int idx);

    int (*is_mat44f)(lua_State *L,
                         int idx);

    //! Get element type
    //! \param l Lua state
    //! \param idx Element idx
    //! \return Element type
    int (*value_type)(lua_State *l,
                      int idx);

    //! Push nil
    //! \param l Lua state
    void (*push_nil)(lua_State *l);

    //! Push uint64_t
    //! \param l Lua state
    //! \param value Value
    void (*push_uint64_t)(lua_State *l,
                     uint64_t value);

    //! Push handler
    //! \param l
    //! \param value
    void (*push_handler)(lua_State *l,
                         handler_t value);

    //! Push int
    //! \param l
    //! \param value Value
    void (*push_int)(lua_State *l,
                     int value);

    //! Push bool
    //! \param l
    //! \param value Value
    void (*push_bool)(lua_State *l,
                      int value);

    //! Push float
    //! \param l
    //! \param value Value
    void (*push_float)(lua_State *l,
                       float value);

    //! Push string
    //! \param l
    //! \param value Value
    void (*push_string)(lua_State *l,
                        const char *value);

    //! Get element value as bool
    //! \param l
    //! \param i Element idx
    //! \return Bool value
    int (*to_bool)(lua_State *l,
                   int i);

    //! Get element value as int
    //! \param l
    //! \param i Element idx
    //! \return Int value
    int (*to_int)(lua_State *l,
                  int i);
    //! Get element value as u64
    //! \param l
    //! \param i Element idx
    //! \return sInt value
    u64 (*to_u64)(lua_State *l,
                  int i);

    //! Get element value as float
    //! \param l
    //! \param i Element idx
    //! \return Float value
    float (*to_float)(lua_State *l,
                  int i);

    //! Get element value as hadler
    //! \param l
    //! \param i Element idx
    //! \return Handler value
    handler_t (*to_handler)(lua_State *l,
                            int i);

    //! Get element value as string
    //! \param l
    //! \param i Element idx
    //! \return String value
    const char *(*to_string)(lua_State *,
                             int i);

    //! Get element value as string and set len
    //! \param l
    //! \param i Element idx
    //! \param len Write string len to this variable
    //! \return String value
    const char *(*to_string_l)(lua_State *l,
                               int i,
                               size_t *len);

    //! Get element value as vec2f
    //! \param l
    //! \param i Element idx
    //! \return Vec2f
    cel_vec2f_t *(*to_vec2f)(lua_State *l,
                             int i);

    //! Get element value as vec3f
    //! \param l
    //! \param i Element idx
    //! \return Vec3f
    cel_vec3f_t *(*to_vec3f)(lua_State *l,
                             int i);

    //! Get element value as vec4f
    //! \param l
    //! \param i Element idx
    //! \return Vec4f
    cel_vec4f_t *(*to_vec4f)(lua_State *l,
                             int i);

    //! Get element value as mat44f
    //! \param l
    //! \param i Element idx
    //! \return Mat44f
    cel_mat44f_t *(*to_mat44f)(lua_State *l,
                               int i);

    //! Get element value as quat
    //! \param l
    //! \param i Element idx
    //! \return Quatf
    cel_quatf_t *(*to_quat)(lua_State *l,
                            int i);

    //! Push vec2f
    //! \param l
    //! \param v Value
    void (*push_vec2f)(lua_State *l,
                       cel_vec2f_t v);

    //! Push vec3f
    //! \param l
    //! \param v Value
    void (*push_vec3f)(lua_State *l,
                       cel_vec3f_t v);

    //! Push vec4f
    //! \param l
    //! \param v Value
    void (*push_vec4f)(lua_State *l,
                       cel_vec4f_t v);

    //! Push mat44f
    //! \param l
    //! \param v Value
    void (*push_mat44f)(lua_State *l,
                        cel_mat44f_t v);

    //! Push quatf
    //! \param l
    //! \param v Value
    void (*push_quat)(lua_State *l,
                      cel_quatf_t v);

    //! Execute lua code
    //! \param str Lua code
    //! \return 0 if execute fail else 1
    int (*execute_string)(const char *str);

    //! Execute resource
    //! \param name Resource name
    void (*execute_resource)(stringid64_t name);

    //! Add module function
    //! \param module Module name
    //! \param name Function name
    //! \param func Function
    void (*add_module_function)(const char *module,
                                const char *name,
                                const lua_CFunction func);

    //! Get game callbacks
    //! \return Lua game callbacks
    const struct game_callbacks *(*get_game_callbacks)();

    //! Execute boot script
    //! \param Boot script name
    void (*execute_boot_script)(stringid64_t name);

    //! Call global function
    //! \param func Function name
    //! \param args Function args format string ('i' = int, 'f' = float)
    //! \param ... Args value
    void (*call_global)(const char *func,
                        const char *args,
                        ...);
};

#endif //CETECH_LUA_TYPES_H
//! \}