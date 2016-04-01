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

### ButtonName(buttonIndex)

Return button name for `buttonIndex` or empty string if `ButtonIndex` is invalid.

Argument      | Type | Description
--------------|------|--------------
`buttonIndex` | int  | Button index

### ButtonState(buttonIndex)

Return `true` if button is down in actual frame.

Argument      | Type | Description
--------------|----- |--------------
`buttonIndex` | int  | Button Index


### ButtonPressed(buttonIndex)

Return `true` if button is not down in last frame and now is.

Argument      | Type | Description
--------------|------|--------------
`buttonIndex` | int  | Button index

### ButtonReleased(buttonIndex)

Return `true` if button is down in last frame and now is not.

Argument      | Type | Description
--------------|------|--------------
`buttonIndex` | int  | Button index
