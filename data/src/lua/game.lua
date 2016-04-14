Game = Game or {}

local quit_btn = Keyboard.ButtonIndex 'q'

function Game:init()
    Log.Info("boot.lua", "init {0}", Application.GetPlatform())

    Log.Info("boot.lua", "info")
    Log.Warning("boot.lua", "warn")
    Log.Error("boot.lua", "error")
    Log.Debug("boot.lua", "debug")

    self.world = World.Create();

    --self.unit = Unit.Spawn(self.world, "unit1");
    World.LoadLevel(self.world, "level1");
    self.level = World.LoadLevel(self.world, "level1",
                 Vec3f.make(0.0, 5.0, 0.0),
                 Vec3f.Zero, Vec3f.Unit);

    self.level_unit = World.LevelUnit(self.world, self.level)

    self.unit1 = World.UnitByName(self.world, self.level, "box1")
    self.unit2 = World.UnitByName(self.world, self.level, "box2")
    self.unit = self.unit1

    self.camera_unit = Unit.Spawn(self.world, "camera");
    self.camera = Camera.GetCamera(self.world, self.camera_unit);

    Unit.Spawn(self.world, "unit11");

    Log.Info("sadsadas", "{0}", self.unit);
    Log.Info("sadsadas", "{0}", self.unit2);

    self.debug = false
    self.capture = false
    self.switch_unit = false
end

function Game:shutdown()
    Log.Info("boot.lua", "shutdown")
    World.Destroy(self.world);
end

function Game:update(dt)
    if Keyboard.Pressed(Keyboard.ButtonIndex('r')) then
        Application.RecompileAll()
        Application.ReloadAll()
    end

    if Keyboard.Pressed(quit_btn) then
        Application.Quit()
    end

    if Keyboard.Pressed(Keyboard.ButtonIndex('f9')) then
      self.debug = not self.debug;
      Renderer.SetDebug(self.debug)
    end

    if Keyboard.Pressed(Keyboard.ButtonIndex('f8')) then
      self.switch_unit = not self.switch_unit
      if self.switch_unit then
        self.unit = self.level_unit
      else
        self.unit = self.unit1
      end
    end

    if Keyboard.Pressed(Keyboard.ButtonIndex('f10')) then
      self.capture = not self.capture;

      if self.capture then
        RenderSystem.BeginCapture()
      else
        RenderSystem.EndCapture()
      end
    end

    if Keyboard.Pressed(Keyboard.ButtonIndex('f11')) then
      RenderSystem.SaveScreenShot("screenshot");
    end

    local m_axis = Mouse.axis("delta")

    local unit_transform = Transform.GetTransform(self.world, self.camera_unit)

    local pos = Transform.GetPosition(self.world, unit_transform)
    local rot = Transform.GetRotation(self.world, unit_transform)

    if Mouse.State(Mouse.ButtonIndex("left") ) then
        rot.X = rot.X + m_axis.X * -0.1;
        rot.Y = rot.Y + m_axis.Y * 0.1;
    end

    if Keyboard.State(Keyboard.ButtonIndex('a')) then
        rot.X = rot.X + 0.02
    end

    if Keyboard.State(Keyboard.ButtonIndex('d')) then
        rot.X = rot.X - 0.02
    end

    if Keyboard.State(Keyboard.ButtonIndex('w')) then
        pos.Z = pos.Z + 0.2
    end

    if Keyboard.State(Keyboard.ButtonIndex('s')) then
        pos.Z = pos.Z - 0.2
    end

    --rot.X = rot.X - 0.005 * dt

    Transform.SetPosition(self.world, unit_transform, pos)
    Transform.SetRotation(self.world, unit_transform, rot)

    World.Update(self.world, dt)
    --print("%f, %f", m_axis.x, m_axis.y)
    --print(dt)
end

function Game:render()
    Renderer.RenderWorld(self.world, self.camera)
end

function foo(value)
    return value
end
