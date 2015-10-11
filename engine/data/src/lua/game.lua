local log = require "cetech/log"

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
end

function Game:shutdown()
    log.info("boot.lua", "shutdown")
end

function Game:update(dt)
    --local m_axis = Mouse.axis()
    --print("%f, %f", m_axis.x, m_axis.y)

    if Keyboard.pressed(quit_btn) then
        print("q")
        Application.quit()
    end

    
    if Package.is_loaded(pkg1) then
        print("unload")c
        Package.unload(pkg1)
    end
end
