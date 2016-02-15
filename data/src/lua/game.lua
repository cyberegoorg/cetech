local Game = Game or {}

--local quit_btn = Keyboard.ButtonIndex 'q'
--local btn = Keyboard.ButtonIndex 'd'

function Game:init()
 --    Log.Info("boot.lua", "init")

  --  Log.Info("boot.lua", "info")
  --  Log.Warning("boot.lua", "warn")
  --  Log.Error("boot.lua", "error")
  --  Log.Debug("boot.lua", "debug")
end

function Game:shutdown()
  --  Log.Info("boot.lua", "shutdown")
end

function Game:update(dt)
--  if Keyboard.ButtonPressed(btn) then
  --  Log.Info("lua", "DOWN.......")
  --end
    --local m_axis = Mouse.axis()
    --print("%f, %f", m_axis.x, m_axis.y)

    --print(dt)

    --if Keyboard.pressed(quit_btn) then
        --print("q")
        --Application.quit()
    --end
end

return Game
