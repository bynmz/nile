#pragma once

#include "gravity_system.hpp"
#include "framework/core/nile_game_object.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <vector>

namespace nile
{
class Vec2FieldSystem
{
public:

void update(
    const GravityPhysicsSystem& physicsSystem,
    NileGameObject::Map& physicsObjs,
    std::vector<NileGameObject*>& vectorField) {

    // For each field line we calculate the net gravitational force for that point in space
    for (auto& vf : vectorField) {
        glm::vec2 direction{};
        for (auto& obj : physicsObjs) {
            if (!obj.second.isVectorField){
                direction += physicsSystem.computeForce(obj.second, *vf);
            }
    }

    // This scales the length of the field line based on the log of the length
    // values were chosen through trial and error based on what looks good
    // and then the field line is rotate to point in the direction of the field
    vf->transform2d.scale.x = 0.005f + 0.045f * glm::clamp(glm::log(glm::length(direction) + 1) / 3.f, 0.f, 1.f);
    vf->transform2d.rotation = atan2(direction.y, direction.x);
    }
    }
};

}
