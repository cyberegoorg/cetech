#ifndef CETECH_LUA_RESOURCE_H
#define CETECH_LUA_RESOURCE_H


namespace resource_lua {
    void *loader(struct ct_vio *input,
                 struct cel_alloc *allocator) {
        const int64_t size = input->size(input->inst);
        char *data = CEL_ALLOCATE(allocator, char, size);
        input->read(input->inst, data, 1, size);

        return data;
    }

    void unloader(void *new_data,
                  struct cel_alloc *allocator) {
        CEL_FREE(allocator, new_data);
    }

    void online(uint64_t name,
                void *data) {
        CEL_UNUSED(name, data);
    }

    void offline(uint64_t name,
                 void *data) {
        CEL_UNUSED(name, data);
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   struct cel_alloc *allocator) {

        CEL_UNUSED(name);

        CEL_FREE(allocator, old_data);

        struct lua_resource *resource = (lua_resource *) new_data;
        char *data = (char *) (resource + 1);

        luaL_loadbuffer(_G.L, data, resource->size, "<unknown>");

        if (lua_pcall(_G.L, 0, 0, 0)) {
            const char *last_error = lua_tostring(_G.L, -1);
            lua_pop(_G.L, 1);
            ct_log_a0.error(LOG_WHERE, "%s", last_error);
        }

        return new_data;
    }

    static const ct_resource_callbacks_t callback = {
            .loader = loader,
            .unloader =unloader,
            .online =online,
            .offline =offline,
            .reloader = reloader
    };
}


#endif //CETECH_LUA_RESOURCE_H
