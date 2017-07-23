
#include <cetech/luasys.h>

#ifdef __cplusplus
extern "C" {
#endif

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

void luasys_push_uint64_t(lua_State *l,
                          uint64_t value);

void luasys_push_handler(lua_State *l,
                         uint32_t value);

void luasys_push_int(lua_State *l,
                     int value);

void luasys_push_uint32_t(lua_State *l,
                          uint32_t value);

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

uint32_t luasys_to_u32(lua_State *l,
                       int i);

uint64_t luasys_to_u64(lua_State *_L,
                       int i);

float luasys_to_float(lua_State *l,
                      int i);

uint32_t luasys_to_handler(lua_State *l,
                           int i);

const char *luasys_to_string(lua_State *,
                             int i);

const char *luasys_to_string_l(lua_State *,
                               int,
                               size_t *);



void luasys_to_vec3f(lua_State *l,
                         int i, float* value);

void luasys_to_vec4f(lua_State *l,
                         int i,float* value);

void luasys_to_mat44f(lua_State *l,
                           int i,float* value);

void luasys_to_quat(lua_State *l,
                        int i,float* value);

void luasys_push_vec3f(lua_State *l,
                       float* value);

void luasys_push_vec4f(lua_State *l,
                       float* value);

void luasys_push_mat44f(lua_State *l,
                        float* value);


void luasys_push_quat(lua_State *l,
                      float* value);

int luasys_execute_string(const char *str);

void luasys_add_module_function(const char *module,
                                const char *name,
                                const lua_CFunction func);

void luasys_add_module_constructor(const char *module,
                                   const lua_CFunction func);

void luasys_execute_resource(uint64_t name);

const struct ct_game_callbacks *luasys_get_game_callbacks();

void luasys_execute_boot_script(uint64_t name);

void luasys_call_global(const char *func,
                        const char *args,
                        ...);

int _is_vec2f(lua_State *L,
              int idx);

int _is_vec3f(lua_State *L,
              int idx);

int _is_vec4f(lua_State *L,
              int idx);
int _is_quat(lua_State *L,
             int idx);
int _is_mat44f(lua_State *L,
               int idx);

#ifdef __cplusplus

}
#endif