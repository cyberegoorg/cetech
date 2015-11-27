# Popis

Modul obsahuje funkce pro obsluhu myši (jasně že počítačové, rozhodně vám neumožní podrobit si krysí národ podobně jako
v [Dishonored][dishonored]).

---

# Ukázka
```lua
local left_btn = Mouse.button_index 'left'

function Game:update(dt)
    local m_axis = Mouse.axis()

    if Mouse.button_state(left_btn) then
        print("%f, %f", m_axis.x, m_axis.y)
    end
end
```

---

# Metody

## button_index
```lua
function Mouse.button_index(button_name)
```

Vrátí index tlačítka který má název `button_name`.

### Parametry

Parametr      | Typ | Popis
--------------|-----|---------------
`button_name` | str | Název tlačítka

### button_name
button_name   | Tlačítko
--------------|------------
`left`        | Levé
`middle`      | Prostřední
`right`       | Pravé

---

## button_name
```lua
function Mouse.button_name(button_index)
```

Vrátí název tlačítka nebo prázdný řetezec pokud je index špatný.

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`button_index` | int | Index tlačítka

---

## button_state

```lua
function Mouse.button_state(button_index)
```

Vrátí `true` pokuď je tlačítko v aktuálním frejmu stisknuté.

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`button_index` | int | Index tlačítka

---

## pressed

```lua
function Mouse.pressed(button_index)
```

Vráti `true` pokud nebylo tlačítko v minulém frejmu stisknuté ale v aktuálním je jinak vrací `false`. 

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`button_index` | int | Index tlačítka

---

## released

```lua
function Mouse.released(button_index)
```

Vráti `true` pokud bylo tlačítko v minulém frejmu stisknuté ale v aktuálním není jinak vrací `false`.

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`button_index` | int | Index tlačítka


## axis

```lua
function Mouse.axis()
```

Vrátí `Vector2` s pozicí kurzoru.

---

[dishonored]: https://cs.wikipedia.org/wiki/Dishonored