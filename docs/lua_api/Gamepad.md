# cetech.Gamepad

Module contain functions for gamepad.

# Example

```lua
local right_a = Gamepad.axis(0, Gamepad.axis_index("right"))
local left_a = Gamepad.axis(0, Gamepad.axis_index("left"))

move_camera(dt, right_a.x, right_a.x, left_a.x, left_a.x)

if Gamepad.button_state(0, Gamepad.button_index("right_shoulder")) then
    fire()
end
```
 
# Supported button name

* `a`, `b`, `x`, `y`
* `back`, `guide`, `start`
* `left_stick`, `right_stick`
* `left_shoulder`, `right_shoulder`
* `dpad_up`, `dpad_down`, `dpad_left`, `dpad_right`
 
# Methods

* Buttons
    * [button_index(name) : string](#button_index-name-string)
    * [button_name( index ) : string](#button_name-index-string)
    * [button_state( gamepad, index ) : bool](#button_state-gamepad-index-bool)
    * [button_pressed( gamepad, index ) : bool](#button_pressed-gamepad-index-bool)
    * [button_released( gamepad, index ) : bool](#button_released-gamepad-index-bool)
* Axis
    * [axis_index( name ) : number](#axis_index-name-number)
    * [axis_name( index ) : string](#axis_name-index-string)
    * [axis( gamepad, index ) : cetech.Vec3f](#axis-gamepad-index-cetechvec3f)

------------------------------------------------------------------------------------------------------------------------

## button_index( name ) : string

Return button index for `name`.

### Returns
* Button index
    
### Arguments
* **string** `name` - Button name 

------------------------------------------------------------------------------------------------------------------------

## button_name( index ) : string

Return button name for `index` or empty string if `index` is invalid.

### Returns
* Button name
    
### Arguments
* **number** `index` - Button index

------------------------------------------------------------------------------------------------------------------------

## button_state( gamepad, index ) : bool

Return `true` if button is down in actual frame.

### Returns
* Button state
    
### Arguments
* **gamepad** `gamepad` - Gamepad id
* **number** `index` - Button index

------------------------------------------------------------------------------------------------------------------------

## button_pressed( gamepad, index ) : bool

Return `true` if button is not down in last frame and now is.

### Returns
* Button pressed state

### Arguments
* **gamepad** `gamepad` - Gamepad id
* **number** `index` - Button index

---

## button_released( gamepad, index ) : bool

Return `true` if button is down in last frame and now is not.

### Returns
* Button pressed state

### Arguments
* **gamepad** `gamepad` - Gamepad id
* **number** `index` - Button index

------------------------------------------------------------------------------------------------------------------------

## axis_index( name ) : number

Return axis index for `axisName`.

### Returns
* Axis index
    
### Arguments
* **string** `name` - Axis name 

------------------------------------------------------------------------------------------------------------------------

## axis_name( index ) : string

Return axis name for `index` or empty string if `index` is invalid.

### Returns
* Axis name
    
### Arguments
* **number** `index` - Button index

------------------------------------------------------------------------------------------------------------------------

## axis( gamepad, index ) : cetech.Vec3f

Return axis values as Vec3f(x, y, 0.0f)

### Returns
* Axis value
    
### Arguments
* **gamepad** `gamepad` - Gamepad id
* **number** `index` - Axis index
