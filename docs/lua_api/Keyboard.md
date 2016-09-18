# cetech.Keyboard

Module contain basic functions for keyboard.


# Example

Press `q` to quit application. 

```lua
local quit_btn = button_index 'q'

function Game:update(dt)
    if Keyboard.ButtonPressed(quit_btn) then
        Application.Quit()
    end
end
``` 


# Button names
* `a`, `b`, `c`, `d`, `e`, `f`, `g`, `h`, `i`, `j`, `k`, `l`, `m`, 
  `n`, `o`, `p`, `q`, `r`, `s`, `t`, `u`, `v`, `w`, `x`, `y`, `z`,
  `0`, `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`

* `insert`, `return`, `escape`, `backspace`, `tab`, `space`, `home`, 
  `pageup`, `delete`, `end`, `pagedown`, `right`, `left`, `down`, `up`


* `insert`, `return`, `escape`, `backspace`, `tab`, `space`, `home`, 
  `pageup`, `delete`, `end`, `pagedown`, `right`, `left`, `down`, `up`

* `lctrl`, `rctrl`
* `lshift`, `rshift`
* `lalt`, `ralt`

# Methods
   
* Button
    * [button_index( name ) : string](#button_index-name-string)
    * [button_name( index ) : string](#button_name-index-string)
    * [button_state( index ) : bool](#button_state-index-bool)
    * [button_pressed( index ) : bool](#button_pressed-index-bool)
    * [button_released( index ) : bool](#button_released-index-bool)

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

