/*
 * ------
 * Copyright (c) 2026 Petr Jirmus
 * All rights reserved.
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace tengine {

struct Transform {
    glm::vec3 position = glm::vec3{0.0f, 0.0f, 0.0f};
    glm::vec3 scale = glm::vec3{1.0f, 1.0f, 1.0f};
    glm::quat rotation = glm::quat{0.0f, 0.0f, 0.0f, 1.0f};

    [[nodiscard]] inline auto toMatrix() const -> glm::mat4 {
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 sc = glm::scale(glm::mat4(1.0f), scale);
        glm::mat4 rot = glm::mat4_cast(rotation);
        return pos * rot * sc;
    }
};

}  // namespace tengine
