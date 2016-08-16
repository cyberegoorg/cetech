#ifndef CETECH_LUA_API_H
#define CETECH_LUA_API_H

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

//==============================================================================
// Typedefs
//==============================================================================

typedef struct lua_State lua_State;

typedef int (*lua_CFunction)(lua_State *L);

//==============================================================================
// Interface
//==============================================================================

struct lua_api_v0 {
    void (*init)();

    void (*shutdown)();

    int (*get_top)(lua_State *);

    void (*remove)(lua_State *, int);

    void (*pop)(lua_State *, int);

    int (*is_nil)(lua_State *, int);

    int (*is_number)(lua_State *, int);

    int (*value_type)(lua_State *, int);

    void (*push_nil)(lua_State *);

    void (*push_bool)(lua_State *, int);

    void (*push_float)(lua_State *, float);

    void (*push_string)(lua_State *, const char *);

    int (*to_bool)(lua_State *, int);

    int (*to_int)(lua_State *, int);

    float (*to_float)(lua_State *, int);

    const char *(*to_string)(lua_State *, int);

    const char *(*to_string_l)(lua_State *, int, size_t *);

    int (*execute_string)(const char *str);

    void (*add_module_function)(const char *module, const char *name, const lua_CFunction func);
};

extern struct lua_api_v0 LUA;

#endif //CETECH_LUA_API_H
