local utils = require("utils")

function hello(whom, what)
    utils.log("Hello, " .. whom .. " from Lua")
    utils.log(what)
end

function add(a, b)
    utils.log("adding " .. a .. " + " .. b)
    if type(a) == "string" or type(b) == "string" then
        return a .. b
    end
    return a + b
end

myprint("Engine call from Lua")
myprint(tostring(strEqual("hello", "hello")))
