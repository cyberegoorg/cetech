require "lua/game"
require "core/cetech/editor"
 
function init()
    print('init............')
    Game:init()
end 

function update(dt) 
  Game:update(dt) 
end 

function shutdown()
  Game:shutdown()
end
