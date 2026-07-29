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

function Player:update(dt)
    utils.log("Player update" .. " " .. dt)
end

return Player
