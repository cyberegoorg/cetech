# cetech.Package

Module contain functions for resource package manage.

# Example

```lua
local main_pkg = 'main'
local monster_pkg = 'monster'

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

# Methods

* [load( package )](#load-package)
* [unload( package )](#unload-package)
* [is_loaded( package ) : bool](#is_loaded-package-bool)
* [flush( package )](#flush-package)

------------------------------------------------------------------------------------------------------------------------

## load( package )

Load all resource that are in the package. If one is already loaded will not reload.

!!! important

    The function does not wait to finish loading.
    For querying whether the package is already loaded use method [**IsLoaded**](#Packageisloaded)
    or you can wait to load using method [**Flush**](#Packageflush)

### Arguments
* `package` : **string**   - Package name

------------------------------------------------------------------------------------------------------------------------

## unload( package )

Load all resource that are in the package.

### Arguments
* `package` : **string**   - Package name

------------------------------------------------------------------------------------------------------------------------

## is_loaded( package ) : bool

Returns `true` if all resource that are in the package loaded in memory and ready to use.


### Arguments
* `package` : **string**   - Package name

------------------------------------------------------------------------------------------------------------------------

## flush( package )

It would wait until they are loaded all the resource that is in the package.

### Arguments
* `package` : **string**   - Package name
