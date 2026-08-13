--[[
-- Copyright (c) 2026 Petr Jirmus
-- All rights reserved.
--
--]]

local log = require("utils").log

local Player = {}

function Player:ready()
    local v = Vec3.new(0.0, 0.0, 1.0)
    self.transform_.position = v
    log("Player ready")
end

function Player:update(dt)
    local tf = self.transform_.position
    tf.x = tf.x + 1.0 * dt

    -- log("Player update" .. " " .. dt)
    -- log("player.transform_.position: [" .. tf.x .. ", " .. tf.y .. ", " .. tf.z .. "]")
end

return Player
