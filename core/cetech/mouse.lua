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

    void (*axis)(uint32_t idx,
                    const uint32_t axis_index, float* value);

    void (*update)();
}
]]

local C = ffi.C
local api = api_system.load("ct_mouse_a0")

--! #### Example
--!
--! ```lua
--! local left_btn = cetech.Mouse.button_index 'left'
--!
--! function Game:update(dt)
--!     local m_axis = Mouse.axis(Mouse.axis_index('absolute'))
--!
--!     if Mouse.state(left_btn) then
--!         print("%f, %f", m_axis.x, m_axis.y)
--!     end
--! end
--! ```
--!
--! #### Axis name
--!
--! * ` ` - Invlaid axis
--! * `absolute` - Abosilute position
--! * `relative` - Relative position since last frame
--!
--! #### Button name
--!
--! * ` ` - Invalid buton
--! * `left` - Left button
--! * `midle` - Midle button
--! * `right` - Right buton
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
    local p = ffi.new("float[3]")
    api.axis(0, index, p)

    return cetech.Vec3f.make(p[0], p[1], 0.0)
end

return Mouse