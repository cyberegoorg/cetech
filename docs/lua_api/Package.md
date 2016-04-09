# Package

Module contain functions for resource package manage.

## Example

```lua
local main_pkg = Package.Create 'main'
local monster_pkg = Package.Create 'monster'

function Game:init()
    Package.Load(monster_pkg)

    Package.Load(main_pkg)
    Package.Flush(main_pkg)

    -- main_pkg loaded
end

function Game:update(dt)
    if Package.IsLoaded(monster_pkg) then
        -- Package loaded
    end
end
```

## Methods

### Load(packageName)

Load all resource that are in the package. If one is already loaded will not reload.

!!! important

    The function does not wait to finish loading.
    For querying whether the package is already loaded use method [**IsLoaded**](#Packageisloaded)
    or you can wait to load using method [**Flush**](#Packageflush)

Argument      | Type | Description
--------------|------|-------------
`packageName` | str  | Package name

### Unload(packageName)

Load all resource that are in the package.

Argument      | Type | Description
--------------|------|-------------
`packageName` | str  | Package name

### IsLoaded(packageName)

Returns `true` if all resource that are in the package loaded in memory and ready to use.

Argument      | Type | Description
--------------|------|-------------
`packageName` | str  | Package name

### Flush(packageName)

It would wait until they are loaded all the resource that is in the package.

Argument      | Type | Description
--------------|------|-------------
`packageName` | str  | Package name

