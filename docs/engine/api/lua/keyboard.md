# Popis
Modul obsahuje funkce pro obsluhu klávesnice.

---

# Ukázka
```lua
local quit_btn = Keyboard.button_index 'q'

function Game:update(dt)
    if Keyboard.pressed(quit_btn) then
        Application.quit()
    end
end
```

---

# Metody

## button_index
```lua
function button_index(button_name)
```

Vrátí index tlačítka který má název `button_name`.

### Parametry

Parametr      | Typ | Popis
--------------|-----|---------------
`button_name` | str | Název tlačítka

---

## button_name
```lua
function button_name(button_index)
```

Vrátí název tlačítka nebo prázdný řetezec pokud je index špatný.

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`button_index` | int | Index tlačítka

---

## button_state
```lua
function button_state(button_index)
```

Vrátí `true` pokud je tlačítka v aktuálním frejmu stisknuté.

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`button_index` | int | Index tlačítka

---

## pressed
```lua
function pressed(button_index)
```

Vráti `true` pokud nebylo tlačítko v minulém frejmu stisknuté.

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`button_index` | int | Index tlačítka

---

## released
```lua
function released(button_index)
```

Vráti `true` pokud bylo tlačítko v minulém frejmu stisknuté ale v aktuálním není.

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`button_index` | int | Index tlačítka

