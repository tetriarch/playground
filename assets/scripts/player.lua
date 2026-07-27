--[[
-- Copyright (c) 2026 Petr Jirmus
-- All rights reserved.
--
--]]

local utils = require("utils")

local Player = {}

function Player:ready()
    utils.log("Player ready")
end

-- TODO: leave it like this until you solve issue #4
-- then remove the self param
function Player:update(self, dt)
    assert(type(dt) == "number", "dt has to be number")

    -- utils.log("Player update" .. " " .. dt)
end

return Player
