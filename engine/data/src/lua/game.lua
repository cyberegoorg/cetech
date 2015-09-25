local log = require "lua/cetech/log"

Game = Game or {}

function Game:init()
    log.info("boot.lua", "init")
    log.info("boot.lua", "info")
    log.warn("boot.lua", "warn")
    log.error("boot.lua", "error")
    log.debug("boot.lua", "debug")
end

function Game:shutdown()

end

function Game:update(dt)

end