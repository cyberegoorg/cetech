# PackageManager

Module contain functions for resource package manage.

## Example

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

## Methods

### Load(packageName)

Load all resource that are in the package. If one is already loaded will not reload.

!!! important

    The function does not wait to finish loading.
    For querying whether the package is already loaded use method [**IsLoaded**](#packagemanagerisloaded)
    or you can wait to load using method [**Flush**](#packagemanagerflush)

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

