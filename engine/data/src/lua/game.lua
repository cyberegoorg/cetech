local log = require "core/cetech/log"

Game = Game or {}

local quit_btn = Keyboard.button_index 'q' 

function Game:init() 
    log.info("boot.lua", "init")
    
    log.info("boot.lua", "info")
    log.warn("boot.lua", "warn")
    log.error("boot.lua", "error")
    log.debug("boot.lua", "debug")
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
end
