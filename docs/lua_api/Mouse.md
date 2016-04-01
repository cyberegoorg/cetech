# Mouse

Module contain functions for mouse (computer mouse because this is not [Dishonored][dishonored])

## Example

```lua
local left_btn = Mouse.ButtonIndex 'left'

function Game:update(dt)
    local m_axis = Mouse.Axis('abs')

    if Mouse.ButtonState(left_btn) then
        print("%f, %f", m_axis.x, m_axis.y)
    end
end 
```

## Methods

### ButtonIndex(buttonName)

Return button index for `buttonName`.

Argument     | Typ | Popis
-------------|-----|----------------------------
`buttonName` | str | [Button name](#button-name)

#### Button name

Name        | Button
------------|--------
`left`      | Left
`middle`    | Midle
`right`     | Right

### ButtonName(ButtonIndex)

Return button name for `buttonIndex` or empty string if `ButtonIndex` is invalid.

Argument       | Typ | Popis
---------------|-----|---------------
`ButtonIndex` | int | Button index

### ButtonState(ButtonIndex)

Return `true` if button is down in actual frame.

Argument      | Typ | Popis
--------------|-----|--------------
`ButtonIndex` | int | Button index

### Pressed(ButtonIndex)

Return `true` if button is not down in last frame and now is.

Argument      | Typ | Popis
--------------|-----|--------------
`ButtonIndex` | int | Button index

### Released(ButtonIndex)

Return `true` if button is down in last frame and now is not.

Argument      | Typ | Popis
--------------|-----|--------------
`ButtonIndex` | int | Button index


[dishonored]: https://www.wikipedia.org/wiki/Dishonored
