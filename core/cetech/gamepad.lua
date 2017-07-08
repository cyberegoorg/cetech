local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
struct ct_gamepad_a0 {
    int (*is_active)(uint32_t idx);

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

    void (*play_rumble)(uint32_t idx,
                        float strength,
                        uint32_t length);

    void (*update)();
}
]]

local C = ffi.C
local api = api_system.load("ct_gamepad_a0")

Gamepad = {}


function Gamepad.is_active(controler)
    return api.is_active(controler)
end

function Gamepad.button_index(name)
    return api.button_index(name)
end

function Gamepad.button_name(index)
    return api.button_name(index)
end

function Gamepad.button_state(controler, index)
    return api.button_state(controler, index) > 0
end

function Gamepad.button_pressed(controler, index)
    return api.button_pressed(controler, index) > 0
end

function Gamepad.button_released(controler, index)
    return api.button_released(controler, index) > 0
end

function Gamepad.axis_index(name)
    return api.axis_index(name)
end

function Gamepad.axis_name(index)
    return api.axis_name(index)
end

function Gamepad.axis(controler, index)
    a = api.axis(controler, index)

    return cetech.Vec2f.make(a.x, a.y)
end

function Gamepad.play_rumble(controler, strength, length)
    api.play_rumble(controler, strength, length)
end

return Gamepad