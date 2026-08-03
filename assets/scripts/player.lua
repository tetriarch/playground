--[[
-- Copyright (c) 2026 Petr Jirmus
-- All rights reserved.
--
--]]

local utils = require("utils")

local Player = {}

function Player:ready()
    local tf = self.transform_.position
    tf.x = 10.0
    tf.y = 0.0
    tf.z = 10.0

    utils.log("player.transform_.position: [" .. tf.x .. ", " .. tf.y .. ", " .. tf.z .. "]")

    utils.log("Player ready")
end

function Player:update(dt)
    local tf = self.transform_.position
    tf.x = tf.x + 1.0 * dt
    utils.log("Player update" .. " " .. dt)
    utils.log("player.transform_.position: [" .. tf.x .. ", " .. tf.y .. ", " .. tf.z .. "]")
end

return Player
