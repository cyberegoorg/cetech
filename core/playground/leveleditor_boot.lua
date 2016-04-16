require "playground/editor_input"
require 'core/fpscamera'

Editor = Editor or {}

function Editor:init()
    EditorInput:Init()

    self.world = World.Create()
    self.camera_unit = Unit.Spawn(self.world, "camera")
    self.camera = Camera.GetCamera(self.world, self.camera_unit)
    self.camera_transform = Transform.GetTransform(self.world, self.camera_unit)
    self.fps_camera = FPSCamera(self.world, self.camera_unit)

    Transform.SetPosition(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, -10))

    self.level = nil

    --Renderer.SetDebug(true)
end

function Editor:shutdown()
    World.Destroy(self.world)
end

function Editor:update(dt)
    local dx = 0
    local dy = 0

    if EditorInput.left then
        dx = EditorInput.delta_x * 0.01
        dy = EditorInput.delta_y * 0.01
    end

    self.fps_camera:update(dt, dx, dy, EditorInput.keyboard.left, EditorInput.keyboard.right, EditorInput.keyboard.up, EditorInput.keyboard.down)
    EditorInput:ResetButtons()

    World.Update(self.world, dt)
end

function Editor:render()
    Renderer.RenderWorld(self.world, self.camera)
end

function Editor:load_level(level)
  self.level = World.LoadLevel(self.world, level)
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
