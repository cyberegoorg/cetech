require "lua/game"

function init()
    Log.Info("lua", "INIT.......")
    Game:init()
end

function update(dt)
  Game:update(dt)
end

function shutdown()
  Game:shutdown()
end
 
