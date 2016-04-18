# Mouse

Module contain functions for mouse (computer mouse because this is not [Dishonored][dishonored])

## Example

```lua
local left_btn = Mouse.ButtonIndex 'left'

function Game:update(dt)
    local m_axis = Mouse.Axis('abs')

    if Mouse.State(left_btn) then
        print("%f, %f", m_axis.x, m_axis.y)
    end
end 
```

## Methods

### index(buttonName)

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

### ButtonName(index)

Return button name for `buttonIndex` or empty string if `index` is invalid.

Argument       | Typ | Popis
---------------|-----|---------------
`index` | int | Button index

### ButtonState(index)

Return `true` if button is down in actual frame.

Argument      | Typ | Popis
--------------|-----|--------------
`index` | int | Button index

### ButtonPressed(index)

Return `true` if button is not down in last frame and now is.

Argument      | Typ | Popis
--------------|-----|--------------
`index` | int | Button index

### ButtonReleased(index)

Return `true` if button is down in last frame and now is not.

Argument      | Typ | Popis
--------------|-----|--------------
`index` | int | Button index


[dishonored]: https://www.wikipedia.org/wiki/Dishonored
