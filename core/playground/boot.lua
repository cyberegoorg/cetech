require "playground/editor_mouse"

Editor = Editor or {}

function Editor:init()
    EditorMouse:Init()

    self.world = World.Create()
    self.camera_unit = Unit.Spawn(self.world, "camera")
    self.camera = Camera.GetCamera(self.world, self.camera_unit)
    self.camera_transform = Transform.GetTransform(self.world, self.camera_unit)

    Transform.SetPosition(self.world, self.camera_transform, Vec3f.make(0.0, 0.0, -10))

    self.level = nil

    --Renderer.SetDebug(true)
end

function Editor:shutdown()
    World.Destroy(self.world)
end

function Editor:update(dt)
    local pos = Transform.GetPosition(self.world, self.camera_transform)
    local rot = Transform.GetRotation(self.world, self.camera_transform)

    if EditorMouse.left then
        rot.X = rot.X + EditorMouse.delta_x * -0.01
        rot.Y = rot.Y + EditorMouse.delta_y * 0.01
    end
    EditorMouse:ResetButtons()

    Transform.SetPosition(self.world, self.camera_transform, pos)
    Transform.SetRotation(self.world, self.camera_transform, rot)

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
