#pragma once

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <array>
#include <cassert>
#include <stdexcept>

namespace nile {

class GravityPhysicsSystem
{

private:
    void stepSimulation(std::vector<NileGameObject*>& physicsObjs, float dt) {
        // Loops through all pairs of objects and applies attractive force between them
        for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA) {
            auto& objA = *iterA;
            for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB) {
                if (iterA == iterB) continue;
                auto& objB = *iterB;

                auto force = computeForce(*objA, *objB);
                objA->rigidBody2d.velocity += dt * -force / objA->rigidBody2d.mass;
                objB->rigidBody2d.velocity += dt * force / objB->rigidBody2d.mass;
            }
        }

        // update each objects position based on its final velocity
        for (auto& obj : physicsObjs) {
            obj->transform2d.translation += dt * obj->rigidBody2d.velocity;
        }
    }

public:
    GravityPhysicsSystem(float strength) : strengthGravity{strength} {}

    const float strengthGravity;

    // dt stands for delta time, and specifies the amount of time to advance the simulation
    // substeps is how many intervals to divide the forward time step in. More substeps result in a 
    // more stable simulation, but takes longer to compute
    void update(std::vector<NileGameObject*>& objs, float dt, unsigned int substeps = 1) {
        const float stepDelta = dt / substeps;
        for (int i = 0; i < substeps; i++) {
            stepSimulation(objs, stepDelta);
        }
    }

    glm::vec2 computeForce(NileGameObject& fromObj, NileGameObject& toObj) const 
    {
        auto offset = fromObj.transform2d.translation - toObj.transform2d.translation;
        float distanceSquared = glm::dot(offset, offset);

        // return 0 if objects are too clost together...
        if (glm::abs(distanceSquared) < 1e-10f) {
            return {.0f, .0f};
        }

        float force = strengthGravity * toObj.rigidBody2d.mass * fromObj.rigidBody2d.mass / distanceSquared;
        return force * offset / glm::sqrt(distanceSquared);

    }
};
}