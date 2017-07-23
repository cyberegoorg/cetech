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

local api = api_system.load("ct_keyboard_a0")

--! #### Example
--!
--! Press `q` to quit application.
--!
--! ```lua
--! local quit_btn = button_index 'q'
--!
--! function Game:update(dt)
--!     if Keyboard.ButtonPressed(quit_btn) then
--!         Application.Quit()
--!     end
--! end
--! ```
--!
--! ### Button names
--! * `a`, `b`, `c`, `d`, `e`, `f`, `g`, `h`, `i`, `j`, `k`, `l`, `m`,
--!   `n`, `o`, `p`, `q`, `r`, `s`, `t`, `u`, `v`, `w`, `x`, `y`, `z`,
--!   `0`, `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`
--!
--! * `insert`, `return`, `escape`, `backspace`, `tab`, `space`, `home`,
--!   `pageup`, `delete`, `end`, `pagedown`, `right`, `left`, `down`, `up`
--!
--!
--! * `insert`, `return`, `escape`, `backspace`, `tab`, `space`, `home`,
--!   `pageup`, `delete`, `end`, `pagedown`, `right`, `left`, `down`, `up`
--!
--! * `lctrl`, `rctrl`
--! * `lshift`, `rshift`
--! * `lalt`, `ralt`
--!
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