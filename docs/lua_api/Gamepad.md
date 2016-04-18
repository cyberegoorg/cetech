# Gamepad

Module contain functions for gamepad.

## Example

```lua
local right_a = Gamepad.Axis(0, Gamepad.AxisIndex("right"))
local left_a = Gamepad.Axis(0, Gamepad.AxisIndex("left"))

move_camera(dt, right_a.X, right_a.Y, left_a.Y, left_a.X)

if Gamepad.ButtonState(0, Gamepad.ButtonIndex("right_shoulder")) then
    fire()
end
```
 
## Methods

### ButtonIndex(buttonName)

Return button index for `buttonName`.

#### Supported button name:

* `a`, `b`, `x`, `y`
* `back`, `guide`, `start`
* `left_stick`, `right_stick`
* `left_shoulder`, `right_shoulder`
* `dpad_up`, `dpad_down`, `dpad_left`, `dpad_right`

Argument     | Type | Description
-------------|------|---------------
`buttonName` | str  | Button name

### ButtonName(index)

Return button name for `index` or empty string if `ButtonIndex` is invalid.

Argument | Type | Description
---------|------|--------------
`index`  | int  | Button index

### ButtonState(gamepad, index)

Return `true` if button is down in actual frame.

Argument  | Type | Description
----------|----- |--------------
`gamepad` | int  | Gamepad id
`index`   | int  | Button Index


### ButtonPressed(gamepad, index)

Return `true` if button is not down in last frame and now is.

Argument  | Type | Description
----------|------|--------------
`gamepad` | int  | Gamepad id
`index`   | int  | Button index

### ButtonReleased(gamepad, index)

Return `true` if button is down in last frame and now is not.

Argument  | Type | Description
----------|------|--------------
`gamepad` | int  | Gamepad id
`index`   | int  | Button index

### AxisIndex(axisName)

Return axis index for `axisName`.

#### Supported axis name:

* `left`, `right`
* `triger`

Argument     | Type | Description
-------------|------|---------------
`axisName` | str  | Axis name

### Axis(gamepad, index) -> Vec3f

Return axis values as Vec3f(x, y, 0.0f)

Argument  | Type | Description
----------|------|---------------
`gamepad` | int  | Gamepad id
`index`   | int  | Axis index