# World

Module contain functions for world  manager.

## Example

```lua
```

## Methods

### Create()

Create new world and return worldId.

### Destroy(worldId)

Load all resource that are in the package.

Argument      | Type | Description
--------------|------|----------------
`worldId`      | int  | WorldId


### Update(worldId, dt)

Update world.

Argument      | Type   | Description
--------------|--------|----------------
`worldId`     | int    | WorldId
`dt`          | number | Delta time

### LoadLevel(worldId, levelName, position?, rotation?, scale?)

Load level from resource to world and return levelID

Argument      | Type     | Description
--------------|----------|--------------------
`worldId`     | int      | WorldId
`levelName`   | str      | Level resource name
`position`    | Vector3f | Position (optional)
`rotation`    | Vector3f | Rotation (optional)
`scale`       | Vector3f | Scale (optional)

### LevelUnit(worldId, levelID)

Return level unit.

Argument      | Type     | Description
--------------|----------|--------------------
`worldId`     | int      | WorldId
`levelID`     | int      | LevelId


### UnitByName(worldId, levelID, name)

Find unit in level by name and return it.

Argument      | Type     | Description
--------------|----------|--------------------
`worldId`     | int      | WorldId
`levelID`     | int      | LevelId
