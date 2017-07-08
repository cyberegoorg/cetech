local api_system  = require("cetech/api_system")

local ffi = require("ffi")

ffi.cdef[[
struct ct_compilator_api;
struct ct_vio;
struct ct_allocator;

typedef int (*ct_resource_compilator_t)(
        const char *filename,
        struct ct_vio *source_vio,
        struct ct_vio *build_vio,
        struct ct_compilator_api *compilator_api);

typedef struct {
    void *(*loader)(struct ct_vio *input,
                    struct ct_allocator *allocator);

    void (*online)(uint64_t name,
                   void *data);

    void (*offline)(uint64_t name,
                    void *data);

    void (*unloader)(void *new_data,
                     struct ct_allocator *allocator);

    void *(*reloader)(uint64_t name,
                      void *old_data,
                      void *new_data,
                      struct ct_allocator *allocator);
} ct_resource_callbacks_t;

struct ct_resource_a0 {

    void (*set_autoload)(int enable);

    void (*register_type)(uint64_t type,
                          ct_resource_callbacks_t callbacks);

    void (*load)(void **loaded_data,
                 uint64_t type,
                 uint64_t *names,
                 size_t count,
                 int force);

    void (*add_loaded)(uint64_t type,
                       uint64_t *names,
                       void **resource_data,
                       size_t count);

    void (*load_now)(uint64_t type,
                     uint64_t *names,
                     size_t count);

    void (*unload)(uint64_t type,
                   uint64_t *names,
                   size_t count);

    void (*reload)(uint64_t type,
                   uint64_t *names,
                   size_t count);

    void (*reload_all)();

    int (*can_get)(uint64_t type,
                   uint64_t names);

    int (*can_get_all)(uint64_t type,
                       uint64_t *names,
                       size_t count);

    void *(*get)(uint64_t type,
                 uint64_t names);

    int (*type_name_string)(char *str,
                            size_t max_len,
                            uint64_t type,
                            uint64_t name);

    void (*compiler_register)(uint64_t type,
                              ct_resource_compilator_t compilator);

    void (*compiler_compile_all)();
};
]]

local C = ffi.C
local api = api_system.load("ct_resource_a0")

ResourceManager = {}

function ResourceManager.compile_all()
    return api.compiler_compile_all ()
end

function ResourceManager.reload_all()
    return api.reload_all()
end

return ResourceManager


