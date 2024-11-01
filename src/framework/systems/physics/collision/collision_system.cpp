#include "collision_system.hpp"

namespace nile
{
 
 SimpleCollisionSystem::SimpleCollisionSystem(){}
 SimpleCollisionSystem::~SimpleCollisionSystem(){}

bool SimpleCollisionSystem::checkCollision1(NileGameObject &one, NileGameObject &two)
{
    // collision x-axis?
    bool collisionX = one.transform2d.translation.x + one.transform2d.scale.x >= two.transform2d.translation.x &&
        two.transform2d.translation.x + two.transform2d.scale.x >= one.transform2d.translation.x;
    // collision y-axis?
    bool collisionY = one.transform2d.translation.y + one.transform2d.scale.y >= two.transform2d.translation.y &&
        two.transform2d.translation.y + two.transform2d.scale.y >= one.transform2d.translation.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

SimpleCollisionSystem::Collision SimpleCollisionSystem::checkCollision2(NileGameObject &one, NileGameObject &two)
{

    // get center point circle first
    glm::vec2 center(one.transform2d.translation + one.ball->radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.transform2d.scale.x / 2.0f, two.transform2d.scale.y / 2.0f);
    glm::vec2 aabb_center(
        two.transform2d.translation.x + aabb_half_extents.x,
        two.transform2d.translation.y + aabb_half_extents.y
    );
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // add clamped value to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // retrieve vector between center circle and closest point AABB and check if length <= radius
    difference = closest - center;
    if (glm::length(difference) <= one.ball->radius)
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

SimpleCollisionSystem::Direction SimpleCollisionSystem::VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),  // up
        glm::vec2(1.0f, 0.0f),  // right
        glm::vec2(0.0f, -1.0f),  // down
        glm::vec2(-1.0f, 0.0f),  // left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    bool res = (Direction)best_match == DOWN;
    return (Direction)best_match;
}

void SimpleCollisionSystem::doCollisions(
        FrameInfo& frameInfo, 
        NileGameObject& ballobj, 
        NileGameObject& player,
        std::vector<GameLevel> Levels,
        unsigned int           Level
        )
{
    for (NileGameObject::id_t id : Levels[Level].bricks)
    {
        auto & obj = frameInfo.gameObjects.at(id);
        if (!obj.Destroyed)
        {
            Collision collision = checkCollision2(ballobj, obj);
            if (std::get<0>(collision)) // if collision is true
            {
                // destory block if not solid
                if (!obj.isSolid)
                    obj.Destroyed = true;
                // collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);
                if (dir == LEFT || dir == RIGHT)  // horizontal collision
                {
                    // reverse horizontal velocity
                    ballobj.rigidBody2d.velocity.x = -ballobj.rigidBody2d.velocity.x;
                    // relocate
                    float penetration = ballobj.ball->radius - std::abs(diff_vector.x);
                    if (dir == LEFT)
                        // move ball to right
                        ballobj.transform2d.translation.x += penetration;
                    else
                        // move ball to left;
                        ballobj.transform2d.translation.x -= penetration;
                }
                else  // vertical collision
                {
                    // reverse vertical velocity
                    ballobj.rigidBody2d.velocity.y = -ballobj.rigidBody2d.velocity.y;
                    // relocate
                    float penetration = ballobj.ball->radius - std::abs(diff_vector.y);
                    if (dir == UP)
                        // move ball back up
                        ballobj.transform2d.translation.y -= penetration;
                    else
                        // move ball back down
                        ballobj.transform2d.translation.y += penetration;
                }
            }
        }
    };
    Collision result = checkCollision2(ballobj, player);
    if (!ballobj.ball->stuck && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = player.transform2d.translation.x + player.transform2d.scale.x / 2.0f;
        float distance = (ballobj.transform2d.translation.x + ballobj.ball->radius) - centerBoard;
        float percentage = distance / (player.transform2d.scale.x / 2.0f);
        // then move accordingly
        float strength = 2.f;
        glm::vec2 oldVelocity = ballobj.rigidBody2d.velocity;
        ballobj.rigidBody2d.velocity.x = .05f * percentage * strength; // .04f is the initial ball velocity
        ballobj.rigidBody2d.velocity.y = -ballobj.rigidBody2d.velocity.y;
        ballobj.rigidBody2d.velocity = glm::normalize(ballobj.rigidBody2d.velocity) * glm::length(oldVelocity);
    }
}

} // namespace nile
