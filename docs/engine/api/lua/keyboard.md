# Keyboard

Module contain functions for keyboard.

---

# Example
```lua
local quit_btn = Keyboard.ButtonIndex 'q'

function Game:update(dt)
    if Keyboard.ButtonPressed(quit_btn) then
        Application.Quit()
    end
end
```

---

# Methods

## ButtonIndex

```lua
function Keyboard.ButtonIndex(buttonName)
end
```

Return button index for `buttonName`.

### Arguments

Argument     | Type | Description
-------------|------|---------------
`buttonName` | str  | Button name

---

## ButtonName
```lua
function Keyboard.ButtonName(buttonIndex)
end
```

Return button name for `buttonIndex` or empty string if `ButtonIndex` is invalid.

### Arguments

Argument      | Type | Description
--------------|------|--------------
`buttonIndex` | int  | Button index

---

## ButtonState
```lua
function Keyboard.ButtonState(buttonIndex)
end
```

Return `true` if button is down in actual frame.

### Arguments

Argument      | Type | Description
--------------|----- |--------------
`buttonIndex` | int  | Button Index

---

## ButtonPressed
```lua
function Keyboard.ButtonPressed(buttonIndex)
end
```

Return `true` if button is not down in last frame and now is.

### Arguments

Argument      | Type | Description
--------------|------|--------------
`buttonIndex` | int  | Button index

---

## ButtonReleased
```lua
function Keyboard.ButtonReleased(buttonIndex)
end
```

Return `true` if button is down in last frame and now is not.

### Arguments

Argument      | Type | Description
--------------|------|--------------
`buttonIndex` | int  | Button index
