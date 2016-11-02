require "lua/game"

local Log = cetech.Log

function init()
    Log.info("lua", "INIT.......")
    Game:init()
end

function update(dt)
    Game:update(dt)
end

function shutdown()
    Game:shutdown()
end

function render()
    Game:render()
end
