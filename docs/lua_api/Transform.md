# cetech.Transform

Module contain functions for transformation

# Example

```lua
```

# Methods

* Component
    * [get( world, entity ) : tranform](#get-world-entity-transform)
    * [has( world, entity ) : bool](#has-world-entity-bool)

* Get
    * [get_position( world, transform ) : Vec3f](#get_position-world-transform-vec3f)
    * [get_rotation( world, transform ) : Quatf](#get_rotation-world-transform-quatf)
    * [get_scale( world, transform ) : Vec3f ](#get_scale-world-transform-vec3f)
    * [get_world_matrix( world, transform ) : Mat44f](#get_world_matrix-world-transform-mat44f)

* Set
    * [set_position( world, transform, position )](#set_position-world-transform-position)
    * [set_rotation( world, transform, rotation )](#set_rotation-world-transform-rotation)
    * [set_scale( world, transform, scale )](#set_scale-world-transform-scale)

------------------------------------------------------------------------------------------------------------------------

## get( world, entity ) : tranform

Get transform from entity or unit.

### Arguments
* **world** `world` - World
* **entity, unit** `entity` - Entity or unit

------------------------------------------------------------------------------------------------------------------------

## has( world, entity ) : bool

Return true if entity has transfom component.

### Arguments
* **world** `world` - World
* **entity, unit** `entity` - Entity or unit

----------------------------------------------------------------------------------------------------------------------

## get_position( world, transform ) : Vec3f

Get position

### Returns
* Position
  
### Arguments
* **world** `world` - World
* **transform** `transform` - Transform instance

------------------------------------------------------------------------------------------------------------------------

## get_rotation( world, transform ) : Quatf

Get rotation

### Returns
* Rotation

### Arguments
* **world** `world` - World
* **transform** `transform` - Transform instance


------------------------------------------------------------------------------------------------------------------------

## get_scale( world, transform ) : Vec3f

Get scale

### Returns
* Scale

### Arguments
* **world** `world` - World
* **transform** `transform` - Transform instance

------------------------------------------------------------------------------------------------------------------------

## get_world_matrix( world, transform ) : Mat44f

Get world matrix.

### Returns
* World matrix

### Arguments
* **world** `world` - World
* **transform** `transform` - Transform instance


------------------------------------------------------------------------------------------------------------------------

## set_position( world, transform, position )

Set position.

### Arguments
* **world** `world` - World
* **transform** `transform` - Transform instance
* **Vec3f** `position` - New position


------------------------------------------------------------------------------------------------------------------------

## set_rotation( world, transform, rotation )

Set rotation.

### Arguments
* **world** `world` - World
* **transform** `transform` - Transform instance
* **Quatf** `rotation` - New rotation

------------------------------------------------------------------------------------------------------------------------

## set_scale( world, transform, scale )

Set scale.

### Arguments
* **world** `world` - World
* **transform** `transform` - Transform instance
* **Vec3f** `scale` - New scale

