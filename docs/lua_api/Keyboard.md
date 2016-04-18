# Keyboard

Module contain functions for keyboard.

## Example

```lua
local quit_btn = Keyboard.ButtonIndex 'q'

function Game:update(dt)
    if Keyboard.ButtonPressed(quit_btn) then
        Application.Quit()
    end
end
``` 
 
## Methods

### ButtonIndex(buttonName)

Return button index for `buttonName`.

Argument     | Type | Description
-------------|------|---------------
`buttonName` | str  | Button name

### ButtonName(index)

Return button name for `index` or empty string if `ButtonIndex` is invalid.

Argument      | Type | Description
--------------|------|--------------
`index` | int  | Button index

### ButtonState(index)

Return `true` if button is down in actual frame.

Argument      | Type | Description
--------------|----- |--------------
`index` | int  | Button Index


### ButtonPressed(index)

Return `true` if button is not down in last frame and now is.

Argument      | Type | Description
--------------|------|--------------
`index` | int  | Button index

### ButtonReleased(index)

Return `true` if button is down in last frame and now is not.

Argument      | Type | Description
--------------|------|--------------
`index` | int  | Button index
