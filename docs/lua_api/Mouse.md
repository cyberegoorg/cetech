# Mouse

Module contain functions for mouse (computer mouse because this is not [Dishonored][dishonored])

## Example

```lua
local left_btn = Mouse.ButtonIndex 'left'

function Game:update(dt)
    local m_axis = Mouse.Axis(Mouse.AxisIndex('absolute'))

    if Mouse.State(left_btn) then
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

#### Supported button name:

* `left` - Left button
* `middle` - Midle button
* `right` - Right button

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


### AxisIndex(axisName)

Return axis index for `axisName`.

#### Supported axis name:

* `absolute` - Abosolute position in window.
* `delta` - Delta move

Argument     | Type | Description
-------------|------|---------------
`axisName` | str  | Axis name

### Axis(index) -> Vec3f

Return axis values as Vec3f(x, y, 0.0f)

Argument  | Type | Description
----------|------|---------------
`index`   | int  | Axis index


[dishonored]: https://www.wikipedia.org/wiki/Dishonored
