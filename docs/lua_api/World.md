# cetech.World

Module contain functions for world  manager.

# Example

```lua
```

# Methods
* Basic
    * [create() : world](#create-world)
    * [destroy( world )](#destroy-world)
    * [update( world, dt )](#update-world-dt)

* Level
    * [load_level( world, name, position?, rotation?, scale? ) : Level](#load_level-world-name-position-rotation-scale-level)
    * [level_unit( world, level) : Unit](#level_unit-world-level-unit)

* Unit
    * [unit_by_name( world, level, name ) : Unit](#unit_by_name-world-level-name-unit)

------------------------------------------------------------------------------------------------------------------------

## create() : world

Create new world and return worldId.

### Returns
* World

------------------------------------------------------------------------------------------------------------------------

## destroy( world )

Load all resource that are in the package.

### Arguments
* `world` : **world**   - World

------------------------------------------------------------------------------------------------------------------------

## update( world, dt )

Update world.

### Arguments
* `world` : **world**   - World
* `dt` : **number**   - delta time

------------------------------------------------------------------------------------------------------------------------

## load_level( world, name, position?, rotation?, scale? ) : Level

Load level from resource to world and return levelID

### Returns
* Level

### Arguments
* `world` : **world**   - World
* `name` : **string**   - Level resource name
* `name` : **string**   - Level resource name
* `position` : **Vec3f**   - Position(optional)
* `rotation` : **Quatf**   - Rotation (optional)
* `scale` : **Vec3f**   - Scale (optional)

------------------------------------------------------------------------------------------------------------------------

## level_unit( world, level) : Unit

Return level unit.

### Returns
* Level unit

### Arguments
* `world` : **world**   - World
* `level` : **level**   - Level

------------------------------------------------------------------------------------------------------------------------

## unit_by_name( world, level, name ) : Unit

Find unit in level by name and return it.

### Returns
* Unit

### Arguments
* `world` : **world**   - World
* `level` : **level**   - Level
* `name` : **string**   - Unit name