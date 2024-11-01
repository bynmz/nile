//#pragma once

#include "apps/app.hpp"

#include "framework/systems/physics/collision/collision_system.hpp"

#include <fstream>
#include <string>
#include <sstream>
#include <vector>

namespace nile
{

class Breakout : public App2D
{
public:
    Breakout();
    ~Breakout() override;

    void loop() override;
    void start() override;

private:
   enum Direction {
        UP,
        RIGHT,
        DOWN,
        LEFT
   };

   typedef std::tuple<bool, Direction, glm::vec2> Collision;

   std::unique_ptr<NileModel> createCircleSprite(NileDevice& device, unsigned int numSides);
   std::unique_ptr<NileModel> createRectangleSprite(NileDevice& device, float width, float height);
    
   void loadGameLevels();

   void loadGameObjects() override;
   void updateBallPos(float dt, unsigned int window_width);
   void action(GLFWwindow* window, NileGameObject &gameObject);
   
   KeyboardMovementController ballController{};

   NileGameObject* player;
   NileGameObject* ballobj;
private:
   std::vector<GameLevel> Levels;
   unsigned int           Level;
};

} // namespace nile