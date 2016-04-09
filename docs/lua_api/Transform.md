# Transform

Module contain functions for transformation

## Example

```lua
```

## Methods

### GetTransform(worldId, entity)

Get transformId from entity or unit.

Argument      | Type | Description
--------------|------|-------------------
`worldId`     | int  | WorldId
`entity`      | int  | Entity or Unit

### HasTransform(worldId, entity)

Return true if entity has transfom component.

Argument      | Type | Description
--------------|------|-------------------
`worldId`     | int  | WorldId
`entity`      | int  | Entity or Unit

### SetPosition(worldId, transformId, position)

Set position.

Argument      | Type     | Description
--------------|----------|-------------------
`worldId`     | int      | WorldId
`transformId` | int      | TransformId
`position`    | Vector3f | Position

### GetPosition(worldId, transformId)

Get position

Argument      | Type     | Description
--------------|----------|-------------------
`worldId`     | int      | WorldId
`transformId` | int      | TransformId

### SetRotation(worldId, transformId, rotation)

Set rotation.

Argument      | Type     | Description
--------------|----------|-------------------
`worldId`     | int      | WorldId
`transformId` | int      | TransformId
`rotation`    | Vector3f | Rotation

### GetRotation(worldId, transformId)

Get rotation

Argument      | Type     | Description
--------------|----------|-------------------
`worldId`     | int      | WorldId
`transformId` | int      | TransformId


### SetScale(worldId, transformId, scale)

Set scale.

Argument      | Type     | Description
--------------|----------|-------------------
`worldId`     | int      | WorldId
`transformId` | int      | TransformId
`scale`       | Vector3f | Scale

### GetScale(worldId, transformId)

Get scale

Argument      | Type     | Description
--------------|----------|-------------------
`worldId`     | int      | WorldId
`transformId` | int      | TransformId

### GetWorldMatrix(worldId, transformId)

Get world matrix.

Argument      | Type     | Description
--------------|----------|-------------------
`worldId`     | int      | WorldId
`transformId` | int      | TransformId