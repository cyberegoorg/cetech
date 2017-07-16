# Modules
* [Application](#application)
* [Module](#module)
* [Level](#level)
* [World](#world)
* [ApiSystem](#apisystem)
* [Material](#material)
* [HashLib](#hashlib)
* [Gamepad](#gamepad)
* [Entity](#entity)
* [Mesh](#mesh)
* [Camera](#camera)
* [Renderer](#renderer)
* [ResourceManager](#resourcemanager)
* [Keyboard](#keyboard)
* [Transform](#transform)
* [Package](#package)
* [Mouse](#mouse)
* [Component](#component)
* [SceneGraph](#scenegraph)

## Application
Module contain functions for Application.



#### Methods
* [Application.quit()](#applicationquit)
* [Application.get_platform()](#applicationget_platform)
* [Application.get_native_platform()](#applicationget_native_platform)


--------------------------------------------------------------------------------

#### Application.quit
```lua
function Application.quit() end
```

Quit application




--------------------------------------------------------------------------------

#### Application.get_platform
```lua
function Application.get_platform() end
```

Get platform



#### Return 
* **string** - Platform

--------------------------------------------------------------------------------

#### Application.get_native_platform
```lua
function Application.get_native_platform() end
```

Get native platform



#### Return 
* **string** - native platform

## Module



#### Methods
* [Module.reload(path)](#modulereload)
* [Module.reload_all()](#modulereload_all)


--------------------------------------------------------------------------------

#### Module.reload
```lua
function Module.reload(path) end
```



#### Arguments 
* `path`


--------------------------------------------------------------------------------

#### Module.reload_all
```lua
function Module.reload_all() end
```





## Level



#### Methods
* [Level.load_level(world, name)](#levelload_level)
* [Level.destroy(world, level)](#leveldestroy)
* [Level.entity_by_id(level, name)](#levelentity_by_id)
* [Level.entity(level)](#levelentity)


--------------------------------------------------------------------------------

#### Level.load_level
```lua
function Level.load_level(world, name) end
```



#### Arguments 
* `world`
* `name`


--------------------------------------------------------------------------------

#### Level.destroy
```lua
function Level.destroy(world, level) end
```



#### Arguments 
* `world`
* `level`


--------------------------------------------------------------------------------

#### Level.entity_by_id
```lua
function Level.entity_by_id(level, name) end
```



#### Arguments 
* `level`
* `name`


--------------------------------------------------------------------------------

#### Level.entity
```lua
function Level.entity(level) end
```



#### Arguments 
* `level`


## World



#### Methods
* [World.create()](#worldcreate)
* [World.destroy(world)](#worlddestroy)
* [World.update(world, dt)](#worldupdate)


--------------------------------------------------------------------------------

#### World.create
```lua
function World.create() end
```





--------------------------------------------------------------------------------

#### World.destroy
```lua
function World.destroy(world) end
```



#### Arguments 
* `world`


--------------------------------------------------------------------------------

#### World.update
```lua
function World.update(world, dt) end
```



#### Arguments 
* `world`
* `dt`


## ApiSystem
Api system



#### Methods
* [ApiSystem.load(name)](#apisystemload)


--------------------------------------------------------------------------------

#### ApiSystem.load
```lua
function ApiSystem.load(name) end
```

Get api


#### Arguments 
* `name`

#### Return 
* **lightuserdata** - Api pointer

## Material



#### Methods
* [Material.set_texture(material, slot_name, texture_name)](#materialset_texture)
* [Material.set_vec4f(material, slot_name, value)](#materialset_vec4f)


--------------------------------------------------------------------------------

#### Material.set_texture
```lua
function Material.set_texture(material, slot_name, texture_name) end
```



#### Arguments 
* `material`
* `slot_name`
* `texture_name`


--------------------------------------------------------------------------------

#### Material.set_vec4f
```lua
function Material.set_vec4f(material, slot_name, value) end
```



#### Arguments 
* `material`
* `slot_name`
* `value`


## HashLib



#### Methods
* [HashLib.hash_murmur2_64(key, len, seed)](#hashlibhash_murmur2_64)
* [HashLib.id64_from_str(str)](#hashlibid64_from_str)


--------------------------------------------------------------------------------

#### HashLib.hash_murmur2_64
```lua
function HashLib.hash_murmur2_64(key, len, seed) end
```



#### Arguments 
* `key`
* `len`
* `seed`


--------------------------------------------------------------------------------

#### HashLib.id64_from_str
```lua
function HashLib.id64_from_str(str) end
```



#### Arguments 
* `str`


## Gamepad
#### Example

```lua
local right_a = Gamepad.axis(0, Gamepad.axis_index("right"))
local left_a = Gamepad.axis(0, Gamepad.axis_index("left"))

move_camera(dt, right_a.x, right_a.x, left_a.x, left_a.x)

if Gamepad.button_state(0, Gamepad.button_index("right_shoulder")) then
    fire()
end
```

#### Axis name

* ` ` - Invlaid axis
* `left` - Left stick
* `right` - Right stick
* `triger` - Trigers

#### Button name

* `a`, `b`, `x`, `y`
* `back`, `guide`, `start`
* `left_stick`, `right_stick`
* `left_shoulder`, `right_shoulder`
* `dpad_up`, `dpad_down`, `dpad_left`, `dpad_right`



#### Methods
* [Gamepad.is_active(controler)](#gamepadis_active)
* [Gamepad.button_index(name)](#gamepadbutton_index)
* [Gamepad.button_name(index)](#gamepadbutton_name)
* [Gamepad.button_state(controler, index)](#gamepadbutton_state)
* [Gamepad.button_pressed(controler, index)](#gamepadbutton_pressed)
* [Gamepad.button_released(controler, index)](#gamepadbutton_released)
* [Gamepad.axis_index(name)](#gamepadaxis_index)
* [Gamepad.axis_name(index)](#gamepadaxis_name)
* [Gamepad.axis(controler, index)](#gamepadaxis)
* [Gamepad.play_rumble(controler, strength, length)](#gamepadplay_rumble)


--------------------------------------------------------------------------------

#### Gamepad.is_active
```lua
function Gamepad.is_active(controler) end
```



#### Arguments 
* `controler`


--------------------------------------------------------------------------------

#### Gamepad.button_index
```lua
function Gamepad.button_index(name) end
```



#### Arguments 
* `name`


--------------------------------------------------------------------------------

#### Gamepad.button_name
```lua
function Gamepad.button_name(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Gamepad.button_state
```lua
function Gamepad.button_state(controler, index) end
```



#### Arguments 
* `controler`
* `index`


--------------------------------------------------------------------------------

#### Gamepad.button_pressed
```lua
function Gamepad.button_pressed(controler, index) end
```



#### Arguments 
* `controler`
* `index`


--------------------------------------------------------------------------------

#### Gamepad.button_released
```lua
function Gamepad.button_released(controler, index) end
```



#### Arguments 
* `controler`
* `index`


--------------------------------------------------------------------------------

#### Gamepad.axis_index
```lua
function Gamepad.axis_index(name) end
```



#### Arguments 
* `name`


--------------------------------------------------------------------------------

#### Gamepad.axis_name
```lua
function Gamepad.axis_name(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Gamepad.axis
```lua
function Gamepad.axis(controler, index) end
```



#### Arguments 
* `controler`
* `index`


--------------------------------------------------------------------------------

#### Gamepad.play_rumble
```lua
function Gamepad.play_rumble(controler, strength, length) end
```



#### Arguments 
* `controler`
* `strength`
* `length`


## Entity



#### Methods
* [Entity.destroy(entity)](#entitydestroy)
* [Entity.spawn(world, name)](#entityspawn)


--------------------------------------------------------------------------------

#### Entity.destroy
```lua
function Entity.destroy(entity) end
```



#### Arguments 
* `entity`


--------------------------------------------------------------------------------

#### Entity.spawn
```lua
function Entity.spawn(world, name) end
```



#### Arguments 
* `world`
* `name`


## Mesh



#### Methods
* [Mesh.get(world, entity)](#meshget)
* [Mesh.is_valid(mesh)](#meshis_valid)
* [Mesh.has(world, entity)](#meshhas)
* [Mesh.get_material(mesh)](#meshget_material)
* [Mesh.set_material(mesh, material)](#meshset_material)


--------------------------------------------------------------------------------

#### Mesh.get
```lua
function Mesh.get(world, entity) end
```



#### Arguments 
* `world`
* `entity`


--------------------------------------------------------------------------------

#### Mesh.is_valid
```lua
function Mesh.is_valid(mesh) end
```



#### Arguments 
* `mesh`


--------------------------------------------------------------------------------

#### Mesh.has
```lua
function Mesh.has(world, entity) end
```



#### Arguments 
* `world`
* `entity`


--------------------------------------------------------------------------------

#### Mesh.get_material
```lua
function Mesh.get_material(mesh) end
```



#### Arguments 
* `mesh`


--------------------------------------------------------------------------------

#### Mesh.set_material
```lua
function Mesh.set_material(mesh, material) end
```



#### Arguments 
* `mesh`
* `material`


## Camera



#### Methods
* [Camera.get(world, entity)](#cameraget)
* [Camera.is_valid(camera)](#camerais_valid)
* [Camera.has(world, camera)](#camerahas)


--------------------------------------------------------------------------------

#### Camera.get
```lua
function Camera.get(world, entity) end
```



#### Arguments 
* `world`
* `entity`


--------------------------------------------------------------------------------

#### Camera.is_valid
```lua
function Camera.is_valid(camera) end
```



#### Arguments 
* `camera`


--------------------------------------------------------------------------------

#### Camera.has
```lua
function Camera.has(world, camera) end
```



#### Arguments 
* `world`
* `camera`


## Renderer



#### Methods
* [Renderer.render_world(world, camera, viewport)](#rendererrender_world)
* [Renderer.set_debug(debug)](#rendererset_debug)


--------------------------------------------------------------------------------

#### Renderer.render_world
```lua
function Renderer.render_world(world, camera, viewport) end
```



#### Arguments 
* `world`
* `camera`
* `viewport`


--------------------------------------------------------------------------------

#### Renderer.set_debug
```lua
function Renderer.set_debug(debug) end
```



#### Arguments 
* `debug`


## ResourceManager



#### Methods
* [ResourceManager.compile_all()](#resourcemanagercompile_all)
* [ResourceManager.reload_all()](#resourcemanagerreload_all)


--------------------------------------------------------------------------------

#### ResourceManager.compile_all
```lua
function ResourceManager.compile_all() end
```





--------------------------------------------------------------------------------

#### ResourceManager.reload_all
```lua
function ResourceManager.reload_all() end
```





## Keyboard
#### Example

Press `q` to quit application.

```lua
local quit_btn = button_index 'q'

function Game:update(dt)
    if Keyboard.ButtonPressed(quit_btn) then
        Application.Quit()
    end
end
```

### Button names
* `a`, `b`, `c`, `d`, `e`, `f`, `g`, `h`, `i`, `j`, `k`, `l`, `m`,
  `n`, `o`, `p`, `q`, `r`, `s`, `t`, `u`, `v`, `w`, `x`, `y`, `z`,
  `0`, `1`, `2`, `3`, `4`, `5`, `6`, `7`, `8`, `9`

* `insert`, `return`, `escape`, `backspace`, `tab`, `space`, `home`,
  `pageup`, `delete`, `end`, `pagedown`, `right`, `left`, `down`, `up`


* `insert`, `return`, `escape`, `backspace`, `tab`, `space`, `home`,
  `pageup`, `delete`, `end`, `pagedown`, `right`, `left`, `down`, `up`

* `lctrl`, `rctrl`
* `lshift`, `rshift`
* `lalt`, `ralt`




#### Methods
* [Keyboard.button_index(name)](#keyboardbutton_index)
* [Keyboard.button_name(index)](#keyboardbutton_name)
* [Keyboard.button_state(index)](#keyboardbutton_state)
* [Keyboard.button_pressed(index)](#keyboardbutton_pressed)
* [Keyboard.button_released(index)](#keyboardbutton_released)


--------------------------------------------------------------------------------

#### Keyboard.button_index
```lua
function Keyboard.button_index(name) end
```



#### Arguments 
* `name`


--------------------------------------------------------------------------------

#### Keyboard.button_name
```lua
function Keyboard.button_name(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Keyboard.button_state
```lua
function Keyboard.button_state(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Keyboard.button_pressed
```lua
function Keyboard.button_pressed(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Keyboard.button_released
```lua
function Keyboard.button_released(index) end
```



#### Arguments 
* `index`


## Transform
dsadas
dasdasda



#### Methods
* [Transform.get(world, entity)](#transformget)
* [Transform.is_valid(transform)](#transformis_valid)
* [Transform.has(world, entity)](#transformhas)
* [Transform.get_position(transform)](#transformget_position)
* [Transform.get_rotation(transform)](#transformget_rotation)
* [Transform.get_scale(transform)](#transformget_scale)
* [Transform.get_world_matrix(transform)](#transformget_world_matrix)
* [Transform.set_position(transform, position)](#transformset_position)
* [Transform.set_rotation(transform, rotation)](#transformset_rotation)
* [Transform.set_scale(transform, scale)](#transformset_scale)


--------------------------------------------------------------------------------

#### Transform.get
```lua
function Transform.get(world, entity) end
```

Get transformation component


#### Arguments 
* `world`
* `entity`

#### Return 
* **lightuserdata** - Transformation component

--------------------------------------------------------------------------------

#### Transform.is_valid
```lua
function Transform.is_valid(transform) end
```

Is transformation component valid?


#### Arguments 
* `transform`

#### Return 
* **boolean** - True if is valid.

--------------------------------------------------------------------------------

#### Transform.has
```lua
function Transform.has(world, entity) end
```

Has entity transformation component?


#### Arguments 
* `world`
* `entity`

#### Return 
* **boolean** - True if has.

--------------------------------------------------------------------------------

#### Transform.get_position
```lua
function Transform.get_position(transform) end
```

Get position.


#### Arguments 
* `transform`

#### Return 
* **cetech.Vec3f** - Position.

--------------------------------------------------------------------------------

#### Transform.get_rotation
```lua
function Transform.get_rotation(transform) end
```

Get rotation.


#### Arguments 
* `transform`

#### Return 
* **cetech.Quatf** - Rotation.

--------------------------------------------------------------------------------

#### Transform.get_scale
```lua
function Transform.get_scale(transform) end
```

Get scale.


#### Arguments 
* `transform`

#### Return 
* **cetech.Vec3f** - Scale.

--------------------------------------------------------------------------------

#### Transform.get_world_matrix
```lua
function Transform.get_world_matrix(transform) end
```

Get world matrix.


#### Arguments 
* `transform`

#### Return 
* **cetech.Mat44f** - World matrix.

--------------------------------------------------------------------------------

#### Transform.set_position
```lua
function Transform.set_position(transform, position) end
```

Set position.


#### Arguments 
* `transform`
* `position`


--------------------------------------------------------------------------------

#### Transform.set_rotation
```lua
function Transform.set_rotation(transform, rotation) end
```

Set rotation.


#### Arguments 
* `transform`
* `rotation`


--------------------------------------------------------------------------------

#### Transform.set_scale
```lua
function Transform.set_scale(transform, scale) end
```

Set scale.


#### Arguments 
* `transform`
* `scale`


## Package



#### Methods
* [Package.load(name)](#packageload)
* [Package.load(name)](#packageload)
* [Package.is_loaded(name)](#packageis_loaded)
* [Package.flush(name)](#packageflush)


--------------------------------------------------------------------------------

#### Package.load
```lua
function Package.load(name) end
```



#### Arguments 
* `name`


--------------------------------------------------------------------------------

#### Package.load
```lua
function Package.load(name) end
```

Load all resource that are in the package. If one is already loaded will not reload.

!!! important

    The function does not wait to finish loading.
    For querying whether the package is already loaded use method [**IsLoaded**](#Packageisloaded)
    or you can wait to load using method [**Flush**](#Packageflush)


#### Arguments 
* `name`


--------------------------------------------------------------------------------

#### Package.is_loaded
```lua
function Package.is_loaded(name) end
```



#### Arguments 
* `name`


--------------------------------------------------------------------------------

#### Package.flush
```lua
function Package.flush(name) end
```



#### Arguments 
* `name`


## Mouse
#### Example

```lua
local left_btn = cetech.Mouse.button_index 'left'

function Game:update(dt)
    local m_axis = Mouse.axis(Mouse.axis_index('absolute'))

    if Mouse.state(left_btn) then
        print("%f, %f", m_axis.x, m_axis.y)
    end
end
```

#### Axis name

* ` ` - Invlaid axis
* `absolute` - Abosilute position
* `relative` - Relative position since last frame

#### Button name

* ` ` - Invalid buton
* `left` - Left button
* `midle` - Midle button
* `right` - Right buton



#### Methods
* [Mouse.button_index(name)](#mousebutton_index)
* [Mouse.button_name(index)](#mousebutton_name)
* [Mouse.button_state(index)](#mousebutton_state)
* [Mouse.button_pressed(index)](#mousebutton_pressed)
* [Mouse.button_released(index)](#mousebutton_released)
* [Mouse.axis_index(name)](#mouseaxis_index)
* [Mouse.axis_name(index)](#mouseaxis_name)
* [Mouse.axis(index)](#mouseaxis)


--------------------------------------------------------------------------------

#### Mouse.button_index
```lua
function Mouse.button_index(name) end
```



#### Arguments 
* `name`


--------------------------------------------------------------------------------

#### Mouse.button_name
```lua
function Mouse.button_name(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Mouse.button_state
```lua
function Mouse.button_state(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Mouse.button_pressed
```lua
function Mouse.button_pressed(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Mouse.button_released
```lua
function Mouse.button_released(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Mouse.axis_index
```lua
function Mouse.axis_index(name) end
```



#### Arguments 
* `name`


--------------------------------------------------------------------------------

#### Mouse.axis_name
```lua
function Mouse.axis_name(index) end
```



#### Arguments 
* `index`


--------------------------------------------------------------------------------

#### Mouse.axis
```lua
function Mouse.axis(index) end
```



#### Arguments 
* `index`


## Component



#### Methods
* [Component.set_property(world, entity, component_type, key, value)](#componentset_property)
* [Component.get_property(world, entity, component_type, key)](#componentget_property)


--------------------------------------------------------------------------------

#### Component.set_property
```lua
function Component.set_property(world, entity, component_type, key, value) end
```



#### Arguments 
* `world`
* `entity`
* `component_type`
* `key`
* `value`


--------------------------------------------------------------------------------

#### Component.get_property
```lua
function Component.get_property(world, entity, component_type, key) end
```



#### Arguments 
* `world`
* `entity`
* `component_type`
* `key`


## SceneGraph



#### Methods
* [SceneGraph.get(world, entity)](#scenegraphget)
* [SceneGraph.is_valid(scene_node)](#scenegraphis_valid)
* [SceneGraph.has(world, entity)](#scenegraphhas)
* [SceneGraph.get_position(scene_node)](#scenegraphget_position)
* [SceneGraph.get_rotation(scene_node)](#scenegraphget_rotation)
* [SceneGraph.get_scale(scene_node)](#scenegraphget_scale)
* [SceneGraph.set_position(scene_node, position)](#scenegraphset_position)
* [SceneGraph.set_rotation(scene_node, rotation)](#scenegraphset_rotation)
* [SceneGraph.set_scale(scene_node, scale)](#scenegraphset_scale)
* [SceneGraph.node_by_name(world, entity, name)](#scenegraphnode_by_name)


--------------------------------------------------------------------------------

#### SceneGraph.get
```lua
function SceneGraph.get(world, entity) end
```



#### Arguments 
* `world`
* `entity`


--------------------------------------------------------------------------------

#### SceneGraph.is_valid
```lua
function SceneGraph.is_valid(scene_node) end
```



#### Arguments 
* `scene_node`


--------------------------------------------------------------------------------

#### SceneGraph.has
```lua
function SceneGraph.has(world, entity) end
```



#### Arguments 
* `world`
* `entity`


--------------------------------------------------------------------------------

#### SceneGraph.get_position
```lua
function SceneGraph.get_position(scene_node) end
```



#### Arguments 
* `scene_node`


--------------------------------------------------------------------------------

#### SceneGraph.get_rotation
```lua
function SceneGraph.get_rotation(scene_node) end
```



#### Arguments 
* `scene_node`


--------------------------------------------------------------------------------

#### SceneGraph.get_scale
```lua
function SceneGraph.get_scale(scene_node) end
```



#### Arguments 
* `scene_node`


--------------------------------------------------------------------------------

#### SceneGraph.set_position
```lua
function SceneGraph.set_position(scene_node, position) end
```



#### Arguments 
* `scene_node`
* `position`


--------------------------------------------------------------------------------

#### SceneGraph.set_rotation
```lua
function SceneGraph.set_rotation(scene_node, rotation) end
```



#### Arguments 
* `scene_node`
* `rotation`


--------------------------------------------------------------------------------

#### SceneGraph.set_scale
```lua
function SceneGraph.set_scale(scene_node, scale) end
```



#### Arguments 
* `scene_node`
* `scale`


--------------------------------------------------------------------------------

#### SceneGraph.node_by_name
```lua
function SceneGraph.node_by_name(world, entity, name) end
```



#### Arguments 
* `world`
* `entity`
* `name`


