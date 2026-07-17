--[[
-- Copyright (c) 2026 Petr Jirmus
-- All rights reserved.
--
--]]

local Player = {}

function Player:ready(self)
    print("ready")
end

function Player:update(self, dt)
    print("update")
end

function Player:postUpdate(self, dt)
    print("postUpdate")
end

return Player
