local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
struct ct_mouse_a0 {
    uint32_t (*button_index)(const char *button_name);

    const char *(*button_name)(const uint32_t button_index);

    int (*button_state)(uint32_t idx,
                        const uint32_t button_index);

    int (*button_pressed)(uint32_t idx,
                          const uint32_t button_index);

    int (*button_released)(uint32_t idx,
                           const uint32_t button_index);

    uint32_t (*axis_index)(const char *axis_name);

    const char *(*axis_name)(const uint32_t axis_index);

    vec2f_t (*axis)(uint32_t idx,
                    const uint32_t axis_index);

    void (*update)();
}
]]

local C = ffi.C
local api = api_system.load("ct_mouse_a0")

Mouse = {}

function Mouse.button_index(name)
    return api.button_index(name)
end

function Mouse.button_name(index)
    return api.button_name(index)
end

function Mouse.button_state(index)
    return api.button_state(0, index) > 0
end

function Mouse.button_pressed(index)
    return api.button_pressed(0, index) > 0
end

function Mouse.button_released(index)
    return api.button_released(0, index) > 0
end

function Mouse.axis_index(name)
    return api.axis_index(name)
end

function Mouse.axis_name(index)
    return api.axis_name(index)
end

function Mouse.axis(index)
    local a = api.axis(0, index)

    return cetech.Vec2f.make(a.x, a.y)
end

return Mouse