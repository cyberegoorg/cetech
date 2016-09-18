# cetech.Mouse

Module contain functions for mouse (computer mouse because this is not [Dishonored][dishonored])

# Example

```lua
local left_btn = cetech.Mouse.button_index 'left'

function Game:update(dt)
    local m_axis = Mouse.axis(Mouse.axis_index('absolute'))

    if Mouse.state(left_btn) then
        print("%f, %f", m_axis.x, m_axis.y)
    end
end 
```

# Axis name

* ` ` - Invlaid axis
* `absolute` - Abosilute position
* `relative` - Relative position since last frame

# Button name

* ` ` - Invalid buton
* `left` - Left button
* `midle` - Midle button
* `right` - Right buton
 
# Methods

* Buttons
    * [button_index(name) : string](#button_index-name-string)
    * [button_name( index ) : string](#button_name-index-string)
    * [button_state( index ) : bool](#button_state-index-bool)
    * [button_pressed( index ) : bool](#button_pressed-index-bool)
    * [button_released( index ) : bool](#button_released-index-bool)
* Axis
    * [axis_index( name ) : number](#axis_index-name-number)
    * [axis_name( index ) : string](#axis_name-index-string)
    * [axis( index ) : cetech.Vec3f](#axis-index-cetechvec3f)

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

## button_state( index ) : bool

Return `true` if button is down in actual frame.

### Returns
* Button state
    
### Arguments
* **number** `index` - Button index

------------------------------------------------------------------------------------------------------------------------

## button_pressed( index ) : bool

Return `true` if button is not down in last frame and now is.

### Returns
* Button pressed state

### Arguments
* **number** `index` - Button index

------------------------------------------------------------------------------------------------------------------------

## button_released( index ) : bool

Return `true` if button is down in last frame and now is not.

### Returns
* Button pressed state

### Arguments
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

## axis( index ) : cetech.Vec3f

Return axis values as Vec3f(x, y, 0.0f)

## axis_name( index ) : string

Return button name for `index` or empty string if `index` is invalid.

### Returns
* Axis value
    
### Arguments
* **number** `index` - Axis index


[dishonored]: https://www.wikipedia.org/wiki/Dishonored
