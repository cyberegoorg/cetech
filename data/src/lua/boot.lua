require "lua/game"
--require "core/cetech/editor"

local btn = Keyboard.ButtonIndex 'd'

function init()
    Log.Info("lua", "INIT.......")

    --Game:init()
end

function update(dt)
  if Keyboard.ButtonPressed(btn) then
    Log.Info("lua", "DOWN.......")
  end

  --print(dt)
  --Game:update(dt)
end

function shutdown()
  --Game:shutdown()
end
