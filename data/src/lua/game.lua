Game = Game or {}

require 'core/fpscamera'

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
    self.fps_camera = FPSCamera(self.world, self.camera_unit)
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


    local dx = 0
    local dy = 0
    if Mouse.State(Mouse.ButtonIndex("left") ) then
        local m_axis = Mouse.axis("delta")
        dx, dy = m_axis.X, m_axis.Y
    end

    local up = Keyboard.State(Keyboard.ButtonIndex('w'))
    local down = Keyboard.State(Keyboard.ButtonIndex('s'))
    local left = Keyboard.State(Keyboard.ButtonIndex('a'))
    local right = Keyboard.State(Keyboard.ButtonIndex('d'))

    self.fps_camera:update(dt, dx * 0.01, dy * 0.01, left, right, up, down)
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
