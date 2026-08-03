--[[
-- Copyright (c) 2026 Petr Jirmus
-- All rights reserved.
--
--]]

local utils = require("utils")

local Player = {}

function Player:ready()
    local tf = self.transform_.position
    local v = Vec3.new(10.0, 0.0, 10.0)
    self.transform_.position = v

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
