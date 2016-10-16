# cetech.Mesh

Module contain functions for mesh component.

# Example

```lua
```

# Methods

* Component
    * [get( world, entity ) : tranform](#get-world-entity-mesh)
    * [has( world, entity ) : bool](#has-world-entity-bool)

* Get
    * [get_material( world, mesh ) : material](#get_material-world-mesh-material)

* Set
    * [set_material( world, mesh, material )](#set_material-world-mesh-material)

------------------------------------------------------------------------------------------------------------------------

## get( world, entity ) : mesh

Get mesh from entity or unit.

### Arguments
* `world`  : **world**          - World
* `entity` : **entity, unit**   - Entity or unit

------------------------------------------------------------------------------------------------------------------------

## has( world, entity ) : bool

Return true if entity has mesh component.

### Arguments
* `world` : **world**           - World
* `entity` : **entity, unit**   - Entity or unit

----------------------------------------------------------------------------------------------------------------------

## get_material( world, mesh ) : material

Get material instance attached to mesh

### Returns
* Material instance
  
### Arguments
* `world` : **world**  - World
* `mesh`  : **mesh**   - Mesh instance

------------------------------------------------------------------------------------------------------------------------

## set_material( world, mesh, material )

Set material for mesh.

### Arguments
* `world`    : **world**      - World
* `mesh`     : **transform**  - Mesh instance
* `material` : **string**     - Material resource name
