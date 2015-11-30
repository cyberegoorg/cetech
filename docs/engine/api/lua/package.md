# Popis

Modul obsahuje funkce pro obsluhu balíčků (`resource package`).

---

# Ukázka

```lua
local main_pkg = Package.create 'main'
local monster_pkg = Package.create 'monster'

function Game:init()
    Package.load(monster_pkg)

    Package.load(main_pkg)
    Package.flush(main_pkg)

    -- main_pkg loaded
end

function Game:update(dt)
    if Package.is_loaded(monster_pkg) then
        -- Package loaded
    end
end
```

---

# Metody

## create
```lua
function create(package_name)
```

Vytvoří id balíčku podle `package_name`.

### Parametry

Parametr       | Typ | Popis
---------------|-----|---------------
`package_name` | str | Název balíčku

---

## load
```lua
function load(package_id)
```

Načte všechny `resource` které jsou v balíčku. Pokud je některý už načtený nebude se znovu načítat.

!!! important

    Funkce nečeká na dokončení načítání. Pro dotazování zda je balíček už načtený slouží metoda [**is_loaded**](#is_loaded)
    nebo je možné počkát na načtení pomocí metody [**flush**](#flush)

### Parametry

Parametr     | Typ        | Popis
-------------|------------|-----------
`package_id` | stringid64 | Id balíčku

---

## unload
```lua
function unload(package_id)
```

Pro všechny `resource` které jsou v balíčku sníží počítadlo a pokud `resource` nikdo nepoužívá smaže ho z paměti.

### Parametry

Parametr     | Typ        | Popis
-------------|------------|-----------
`package_id` | stringid64 | Id balíčku

---

## is_loaded
```lua
function is_loaded(package_id)
```

Vrátí `true` pokud jsou všechny `resource` které jsou v balíčku načteny v paměti a připraveny k používaní.

### Parametry

Parametr     | Typ        | Popis
-------------|------------|-----------
`package_id` | stringid64 | Id balíčku

---

## flush
```lua
function flush(package_id)
```

### Parametry

Počká dokud nejsou načteny všechny `resource` které jsou v balíčku.

Parametr     | Typ        | Popis
-------------|------------|-----------
`package_id` | stringid64 | Id balíčku

---

