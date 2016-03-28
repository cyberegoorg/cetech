Game = Game or {}

local quit_btn = Keyboard.ButtonIndex 'q'
local btn = Keyboard.ButtonIndex 'd'

function Game:init()
    Log.Info("boot.lua", "init {0}", Application.GetPlatform())

    Log.Info("boot.lua", "info")
    Log.Warning("boot.lua", "warn")
    Log.Error("boot.lua", "error")
    Log.Debug("boot.lua", "debug")

    Game.world = World.Create();
end

function Game:shutdown()
    Log.Info("boot.lua", "shutdown")
    World.Destroy(Game.world);
end

function Game:update(dt)
    --Log.Info("boot.lua", "init")
    
    if Keyboard.ButtonPressed(btn) then
        Log.Info("lua", "DOWN.......")
    end

    if Keyboard.ButtonPressed(quit_btn) then
        Application.Quit()
    end

    World.Update(Game.world)
    --local m_axis = Mouse.axis()
    --print("%f, %f", m_axis.x, m_axis.y)
    --print(dt)
end
