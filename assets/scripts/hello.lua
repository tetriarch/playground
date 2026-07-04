local utils = require("utils")
function hello(whom, what)
    print("Hello, " .. whom .. " from Lua")
    print(what)
end

function add(a, b)
    print("adding " .. a .. " + " .. b)
    if type(a) == "string" or type(b) == "string" then
        return a .. b
    end
    return a + b
end

myprint("Engine call from Lua")
