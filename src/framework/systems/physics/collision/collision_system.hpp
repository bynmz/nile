#pragma once

#include "framework/core/nile_frame_info.hpp"

// libs

// std
#include <vector>

namespace nile {

class SimpleCollisionSystem
{
private:
   enum Direction {
        UP,
        RIGHT,
        DOWN,
        LEFT
   };

   typedef std::tuple<bool, Direction, glm::vec2> Collision;

   bool checkCollision1(NileGameObject &one, NileGameObject &two); //  AABB - AABB collision

   Collision checkCollision2(NileGameObject &one, NileGameObject &two); //  AABB - Circle collision

   Direction VectorDirection(glm::vec2 target);

public:
    SimpleCollisionSystem(/* args */);
    ~SimpleCollisionSystem();

    SimpleCollisionSystem(const SimpleCollisionSystem &) = delete;
    SimpleCollisionSystem &operator=(const SimpleCollisionSystem &) = delete;

    void doCollisions(
        FrameInfo& frameInfo, 
        NileGameObject& ballobj, 
        NileGameObject& player,
        std::vector<GameLevel> Levels,
        unsigned int           Level
    );
};

}