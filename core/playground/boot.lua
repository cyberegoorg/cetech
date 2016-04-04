Editor = Editor or {}

function Editor:init()
    self.world = World.Create()
    self.camera_unit = UnitManager.Spawn(self.world, "camera")
    self.camera = Camera.GetCamera(self.world, self.camera_unit)

    self.level = nil
end

function Editor:shutdown()
    World.Destroy(self.world)
end

function Editor:update(dt)
    World.Update(self.world)
end

function Editor:render()
    RenderSystem.RenderWorld(self.world, self.camera)
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
