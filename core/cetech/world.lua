local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
typedef struct {
    void (*on_created)(struct ct_world world);

    void (*on_destroy)(struct ct_world world);

    void (*on_update)(struct ct_world world,
                                         float dt);
} ct_world_callbacks_t;

struct ct_world {
    uint32_t h;
};

struct ct_world_a0 {

    //! Register world calbacks
    //! \param clb Callbacks
    void (*register_callback)(ct_world_callbacks_t clb);

    //! Create new world
    //! \return New world
    struct ct_world (*create)();

    //! Destroy world
    //! \param world World
    void (*destroy)(struct ct_world world);

    //! Update world
    //! \param world World
    //! \param dt Delta time
    void (*update)(struct ct_world world,
                   float dt);
};
]]

local C = ffi.C

local api = api_system.load("ct_world_a0")

World = {}

function World.create()
    return api.create()
end

function World.destroy(world)
    return api.destroy(world)
end

function World.update(world, dt)
    return api.update(world, dt)
end

return World