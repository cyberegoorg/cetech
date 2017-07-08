for k, v in pairs(cetech) do _G[k] = v end

require 'cetech/world'
require 'cetech/entity'
require 'cetech/transform'
require 'cetech/scenegraph'
require 'cetech/keyboard'
require 'cetech/mouse'
require 'cetech/gamepad'
require 'cetech/mesh'
require 'cetech/material'
require 'cetech/renderer'
require 'cetech/level'
require 'cetech/application'
require 'cetech/resource'
require 'cetech/component'

require "playground/editor_input"
require 'core/fpscamera'

Editor = Editor or {}

function Editor:init()
    EditorInput:Init()

    self.viewport = 0 --Renderer.GetViewport("default")
    self.world = World.create()
    self.camera_entity = Entity.spawn(self.world, "camera")
    --self.camera = Camera.get(self.world, self.camera_entity)
    self.camera_transform = Transform.get(self.world, self.camera_entity)
    self.fps_camera = FPSCamera(self.world, self.camera_entity, true)

    Transform.set_position(self.camera_transform, Vec3f.make(0.0, 0.0, 10))

    self.level = nil

    --self:load_level("level1")

    --Renderer.SetDebug(true)
end

function Editor:shutdown()
    World.destroy(self.world)
end

function Editor:update(dt)
    local dx = 0
    local dy = 0

    if EditorInput.mouse.left then
        dx = EditorInput.mouse.dx * -1
        dy = EditorInput.mouse.dy * 1
    end

    local leftdown = 0.0
    local updown = 0.0
    if EditorInput.keyboard.left then leftdown = -10.0 end
    if EditorInput.keyboard.right then leftdown = 10.0 end
    if EditorInput.keyboard.up then updown = 10.0 end
    if EditorInput.keyboard.down then updown = -10.0 end
    EditorInput:ResetButtons()

    self.fps_camera:update(dt, dx, dy, updown, leftdown)

    --World.update(self.world, dt)
end

function Editor:render()
    Renderer.render_world(self.world, self.fps_camera.camera, self.viewport)
end

function Editor:load_level(level)
    Log.info("leveleditor", "load level {0}", level)

    if self.level then
        Level.destroy(self.world, self.level)
    end

    self.level = Level.load_level(self.world, level)
end

function init()
    Editor:init()
end

function update(dt)
    Editor:update(dt)
end

function shutdown()
    Editor:shutdown()
end

function render()
    Editor:render()
end

--cetech.Renderer.set_debug(false)
--
--if orig_update == nil then
--    orig_update = Editor.update
--end
--
--function Editor:update(dt)
--    orig_update(dt)
--end

