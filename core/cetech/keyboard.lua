local ffi = require("ffi")
local api_system  = require("cetech/api_system")

ffi.cdef[[
struct ct_keyboard_a0 {
    uint32_t (*button_index)(const char *button_name);

    const char *(*button_name)(const uint32_t button_index);

    int (*button_state)(uint32_t idx,
                        const uint32_t button_index);

    int (*button_pressed)(uint32_t idx,
                          const uint32_t button_index);

    int (*button_released)(uint32_t idx,
                           const uint32_t button_index);

    void (*update)();
};
]]

local C = ffi.C
local api = api_system.load("ct_keyboard_a0")

Keyboard = {}

function Keyboard.button_index(name)
    return api.button_index(name)
end

function Keyboard.button_name(index)
    return api.button_name(index)
end

function Keyboard.button_state(index)
    return api.button_state(0, index) > 0
end

function Keyboard.button_pressed(index)
    return api.button_pressed(0, index) > 0
end

function Keyboard.button_released(index)
    return api.button_released(0, index) > 0
end

return Keyboard