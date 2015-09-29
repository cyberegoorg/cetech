local log = require "lua/cetech/log"

Game = Game or {}

local pkg1 = Package.create("pkg1")
local quit_btn = Keyboard.button_index 'q'

function Game:init()
    log.info("boot.lua", "init")
    
    log.info("boot.lua", "info")
    log.warn("boot.lua", "warn")
    log.error("boot.lua", "error")
    log.debug("boot.lua", "debug")

    Package.load(pkg1)
    Package.unload(pkg1)
end

function Game:shutdown()
    log.info("boot.lua", "shutdown")
end

function Game:update(dt)
    if Keyboard.pressed(quit_btn) then
        print("q")
        Device.quit()
    end
end