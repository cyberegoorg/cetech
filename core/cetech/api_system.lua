local ffi = require("ffi")

ffi.cdef[[
struct ct_api_entry {
    void *entry;
    void *api;
};

struct ct_api_a0 {
    void (*register_api)(const char *name,
                         void *api);

    int (*exist)(const char *name);

    struct ct_api_entry (*first)(const char *name);

    struct ct_api_entry (*next)(struct ct_api_entry *entry);
};

struct ct_api_a0 *ct_api_get();

typedef struct vec3f_s {
    union {
        float f[3];
        struct {
            float x;
            float y;
            float z;
        };
    };
} vec3f_t;

typedef struct vec4f_s {
    union {
        float f[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
} vec4f_t;

//==============================================================================
// Quaternion
//==============================================================================

typedef vec4f_t quatf_t;


//==============================================================================
// Matrix
//==============================================================================

typedef struct mat33f_s {
    union {
        float f[3 * 3];
        struct {
            vec3f_t x;
            vec3f_t y;
            vec3f_t z;
        };
    };
} mat33f_t;


typedef struct mat44f_s {
    union {
        float f[4 * 4];
        struct {
            vec4f_t x;
            vec4f_t y;
            vec4f_t z;
            vec4f_t w;
        };
    };
} mat44f_t;
]]

local C = ffi.C
local api = C.ct_api_get()

--! Api system
ApiSystem = {}

--! Get api
--! \param name string Api name
--! \return lightuserdata Api pointer
function ApiSystem.load(name)
    entry = api.first(name)
    return ffi.cast("struct " .. name .. "*", entry.api)
end

return ApiSystem