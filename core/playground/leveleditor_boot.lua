for k, v in pairs(cetech) do _G[k] = v end

require "playground/editor_input"
require 'core/fpscamera'

Editor = Editor or {}

function Editor:init()
    EditorInput:Init()

    self.viewport = 0 --Renderer.GetViewport("default")
    self.world = World.create()
    self.camera_unit = Unit.spawn(self.world, "camera")
    --self.camera = Camera.get(self.world, self.camera_unit)
    self.camera_transform = Transform.get(self.world, self.camera_unit)
    self.fps_camera = FPSCamera(self.world, self.camera_unit, true)

    Transform.set_position(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, 10))

    self.level = nil

    --Renderer.SetDebug(true)
end

function Editor:shutdown()
    World.destroy(self.world)
end

function Editor:update(dt)
    local dx = 0
    local dy = 0

    if EditorInput.mouse.left then
        dx = EditorInput.mouse.dx * -0.1
        dy = EditorInput.mouse.dy * 0.1
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
    Renderer.render_world(self.world, self.camera, self.viewport)
end

function Editor:load_level(level)
    Log.info("leveleditor", "load level {0}", level)
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
