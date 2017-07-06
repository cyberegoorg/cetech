#include <cetech/celib/allocator.h>
#include <cetech/kernel/hash.h>
#include <cetech/modules/entity.h>
#include <cetech/modules/luasys.h>
#include <cetech/kernel/api_system.h>
#include "../luasys_private.h"

#define API_NAME "Entity"

CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_hash_a0);

static int _entity_spawn(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    const char *name = luasys_to_string(l, 2);

    uint64_t nameid = ct_hash_a0.id64_from_str(name);

    luasys_push_handler(l, ct_entity_a0.spawn(w, nameid).h);
    return 1;
}

static int _entity_destroy(lua_State *l) {
    struct ct_world w = {.h = luasys_to_handler(l, 1)};
    struct ct_entity entity = {.h = luasys_to_handler(l, 2)};

    ct_entity_a0.destroy(w, &entity, 1);
    return 1;
}


void _register_lua_entity_api(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_entity_a0);
    CETECH_GET_API(api, ct_hash_a0);

    luasys_add_module_function(API_NAME, "spawn", _entity_spawn);
    luasys_add_module_function(API_NAME, "destroy", _entity_destroy);
}