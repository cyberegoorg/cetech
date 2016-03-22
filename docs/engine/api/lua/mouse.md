# Mouse

Module contain functions for mouse (For computer mouse because this is not [Dishonored][dishonored])

---

# Example

```lua
local left_btn = Mouse.ButtonIndex 'left'

function Game:update(dt)
    local m_axis = Mouse.Axis()

    if Mouse.ButtonState(left_btn) then
        print("%f, %f", m_axis.x, m_axis.y)
    end
end
```

---

# Methods

## ButtonIndex
```lua
function Mouse.ButtonIndex(buttonName)
end
```

Return button index for `buttonName`.

### Arguments

Argument     | Typ | Popis
-------------|-----|--------------
`buttonName` | str | Button name

### buttonName

buttonName | Tlačítko
-----------|------------
`left`     | Levé
`middle`   | Prostřední
`right`    | Pravé

---

## buttonName
```lua
function Mouse.buttonName(ButtonIndex)
end
```

Return button name for `buttonIndex` or empty string if `ButtonIndex` is invalid.

### Arguments

Argument       | Typ | Popis
---------------|-----|---------------
`ButtonIndex` | int | Button index

---

## ButtonState

```lua
function Mouse.ButtonState(ButtonIndex)
end
```

Return `true` if button is down in actual frame.

### Arguments

Argument      | Typ | Popis
--------------|-----|--------------
`ButtonIndex` | int | Button index

---

## pressed

```lua
function Mouse.pressed(ButtonIndex)
end
```

Return `true` if button is not down in last frame and now is.

### Arguments

Argument      | Typ | Popis
--------------|-----|--------------
`ButtonIndex` | int | Button index

---

## Released

```lua
function Mouse.Released(ButtonIndex)
end
```

Return `true` if button is down in last frame and now is not.

### Arguments

Argument      | Typ | Popis
--------------|-----|--------------
`ButtonIndex` | int | Button index

---

[dishonored]: https://www.wikipedia.org/wiki/Dishonored
