Game = Game or {}

local quit_btn = Keyboard.ButtonIndex 'q'
local btn = Keyboard.ButtonIndex 'd'

function Game:init()
    Log.Info("boot.lua", "init {0}", Application.GetPlatform())

    Log.Info("boot.lua", "info")
    Log.Warning("boot.lua", "warn")
    Log.Error("boot.lua", "error")
    Log.Debug("boot.lua", "debug")

    self.world = World.Create();

    self.unit = UnitManager.Spawn(self.world, "unit1");
    self.debug = false
    self.capture = false
end

function Game:shutdown()
    Log.Info("boot.lua", "shutdown")
    World.Destroy(self.world);
end

function Game:update(dt)
    --Log.Info("boot.lua", "init")

    if Keyboard.ButtonPressed(quit_btn) then
        Application.Quit()
    end

    local m_axis = Mouse.axis("delta")

    local pos = Transformation.GetPosition(self.world, self.unit)
    local rot = Transformation.GetRotation(self.world, self.unit)

    if Keyboard.ButtonPressed(Keyboard.ButtonIndex('f9')) then
      self.debug = not self.debug;
      RenderSystem.SetDebug(self.debug)
    end

    if Keyboard.ButtonPressed(Keyboard.ButtonIndex('f10')) then
      self.capture = not self.capture;

      if self.capture then
        RenderSystem.BeginCapture()
      else
        RenderSystem.EndCapture()
      end
    end

    if Keyboard.ButtonPressed(Keyboard.ButtonIndex('f11')) then
      RenderSystem.SaveScreenShot("screenshot");
    end

    if Mouse.ButtonState(Mouse.ButtonIndex("left") ) then
        rot.X = rot.X + m_axis.X * -0.1;
        rot.Y = rot.Y + m_axis.Y * 0.1;
    end

    if Keyboard.ButtonState(Keyboard.ButtonIndex('a')) then
        rot.X = rot.X + 0.02
    end

    if Keyboard.ButtonState(Keyboard.ButtonIndex('d')) then
        rot.X = rot.X - 0.02
    end

    if Keyboard.ButtonState(Keyboard.ButtonIndex('w')) then
        pos.Z = pos.Z + 0.2
    end

    if Keyboard.ButtonState(Keyboard.ButtonIndex('s')) then
        pos.Z = pos.Z - 0.2
    end

    --rot.X = rot.X - 0.005

    Transformation.SetPosition(self.world, self.unit, pos)
    Transformation.SetRotation(self.world, self.unit, rot)

    World.Update(self.world)
    --print("%f, %f", m_axis.x, m_axis.y)
    --print(dt)
end

function Game:render()
    RenderSystem.RenderWorld(self.world)
end

function foo(value)
    return value
end
