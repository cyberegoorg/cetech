require "lua/game"
 
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

-- TODO: move to core
function autocomplite_list()
    local t = {}

    for class_name,class in pairs(_G) do
        if type(class) == 'table' then
            for function_name,fce in pairs(class) do
                if type(fce) == 'function' then
                    local k = class_name .. '.' .. function_name
                    t[k] = 'function'
               end
            end
        end
    end

    Application.console_send({
        type = "autocomplete_list",
        list = t
    })
end
