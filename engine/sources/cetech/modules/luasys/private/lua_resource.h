#ifndef CETECH_LUA_RESOURCE_H
#define CETECH_LUA_RESOURCE_H


#include <cetech/kernel/fs.h>

void *lua_resource_loader(struct vio *input,
                          struct allocator *allocator) {
    const int64_t size = vio_api_v0.size(input);
    char *data = CETECH_ALLOCATE(allocator, char, size);
    vio_api_v0.read(input, data, 1, size);

    return data;
}

void lua_resource_unloader(void *new_data,
                           struct allocator *allocator) {
    CETECH_DEALLOCATE(allocator, new_data);
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
    CETECH_DEALLOCATE(allocator, old_data);

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


#endif //CETECH_LUA_RESOURCE_H
