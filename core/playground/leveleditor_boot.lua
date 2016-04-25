require "playground/editor_input"
require 'core/fpscamera'

Editor = Editor or {}

function Editor:init()
    EditorInput:Init()

    self.viewport = Renderer.GetViewport("default")
    self.world = World.Create()
    self.camera_unit = Unit.Spawn(self.world, "camera")
    self.camera = Camera.GetCamera(self.world, self.camera_unit)
    self.camera_transform = Transform.GetTransform(self.world, self.camera_unit)
    self.fps_camera = FPSCamera(self.world, self.camera_unit, true)

    Transform.SetPosition(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, 10))

    self.level = nil

    --Renderer.SetDebug(true)
end

function Editor:shutdown()
    World.Destroy(self.world)
end

function Editor:update(dt)
    local dx = 0
    local dy = 0

    if EditorInput.mouse.left then
        dx = EditorInput.mouse.dx * 0.01
        dy = EditorInput.mouse.dy * 0.01
    end

    local leftdown = 0.0
    local updown = 0.0
    if EditorInput.keyboard.left then leftdown = -1.0 end
    if EditorInput.keyboard.right then leftdown = 1.0 end
    if EditorInput.keyboard.up then updown = 1.0 end
    if EditorInput.keyboard.down then updown = -1.0 end
    EditorInput:ResetButtons()

    self.fps_camera:update(dt, dx, dy, updown, leftdown)

    World.Update(self.world, dt)
end

function Editor:render()
    Renderer.RenderWorld(self.world, self.camera, self.viewport)
end

function Editor:load_level(level)
    Log.Info("leveleditor", "load level {0}", level)
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
