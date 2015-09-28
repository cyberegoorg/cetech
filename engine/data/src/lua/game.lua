local log = require "lua/cetech/log"

Game = Game or {}

local pkg1 = Package.create("pkg1")
function Game:init()
    log.info("boot.lua", "init")
    
    log.info("boot.lua", "info")
    log.warn("boot.lua", "warn")
    log.error("boot.lua", "error")
    log.debug("boot.lua", "debug")

    print(#pkg1)
    print(pkg1[1])
    print(pkg1[2])
    Package.load(pkg1)
end

function Game:shutdown()
    log.info("boot.lua", "shutdown")
end

local quit_btn = Keyboard.button_index 'q'
function Game:update(dt)
    if Keyboard.pressed(quit_btn) then
        Device.quit()
    end
end