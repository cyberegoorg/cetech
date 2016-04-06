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

    return t
end