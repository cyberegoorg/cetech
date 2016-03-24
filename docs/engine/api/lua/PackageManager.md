# PackageManager

Module contain functions for resource package manage.

---

# Example

```lua
local main_pkg = PackageManager.Create 'main'
local monster_pkg = PackageManager.Create 'monster'

function Game:init()
    PackageManager.Load(monster_pkg)

    PackageManager.Load(main_pkg)
    PackageManager.Flush(main_pkg)

    -- main_pkg loaded
end

function Game:update(dt)
    if PackageManager.IsLoaded(monster_pkg) then
        -- Package loaded
    end
end
```

---

# Methods

---

## PackageManager.Load
```lua
function PackageManager.Load(packageName)
end
```

Load all resource that are in the package. If one is already loaded will not reload.

!!! important

    The function does not wait to finish loading.
    For querying whether the package is already loaded use method [**is_loaded**](#IsLoaded)
    or you can wait to load using method [**PackageManager.Flush**](#Flush)

### Arguments

Argument      | Type | Description
--------------|------|-------------
`packageName` | str  | Package name

---

## PackageManager.Unload
```lua
function PackageManager.Unload(packageName)
end
```

Load all resource that are in the package.

### Arguments

Argument      | Type | Description
--------------|------|-------------
`packageName` | str  | Package name

---

## IsLoaded
```lua
function PackageManager.IsLoaded(packageName)
end
```

Returns `true` if all resource that are in the package loaded in memory and ready to use.

### Arguments

Argument      | Type | Description
--------------|------|-------------
`packageName` | str  | Package name

---

## PackageManager.Flush
```lua
function PackageManager.Flush(packageName)
end
```

It would wait until they are loaded all the resource that is in the package.

### Arguments

Argument      | Type | Description
--------------|------|-------------
`packageName` | str  | Package name

---

