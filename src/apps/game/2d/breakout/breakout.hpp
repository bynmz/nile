#include "apps/app.hpp"

#include <fstream>
#include <string>
#include <sstream>
#include <vector>

namespace nile
{

struct GameLevel
{
void load(
    NileDevice& device, 
    NileGameObjectManager& gom, 
    const std::string& file, 
    unsigned int levelWidth, 
    unsigned int levelHeight,
    bool isCurrentLevel = false);
bool isCompleted();

void init(
    NileDevice& device, 
    NileGameObjectManager& gom, 
    const std::vector<std::vector<unsigned int>>& tileData,
    unsigned int levelWidth, unsigned int levelHeight, bool isCurrentLevel);
    std::vector<NileGameObject::id_t> bricks;
};

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

   std::unique_ptr<NileModel>  createCircleSprite(NileDevice& device, unsigned int numSides);
   std::unique_ptr<NileModel>  createRectangleSprite(NileDevice& device, float width, float height);
    
   void loadGameLevels();

   void loadGameObjects() override;
   void updateBallPos(float dt, unsigned int window_width);
   void action(GLFWwindow* window, NileGameObject &gameObject);
   Collision checkCollision(NileGameObject& one, NileGameObject &two);
   void doCollisions(FrameInfo frameInfo);
   
   Direction VectorDirection(glm::vec2 target);

   KeyboardMovementController ballController{};

   NileGameObject* player;
   NileGameObject* ballobj;
private:
   std::vector<GameLevel> Levels;
   unsigned int           Level;
};

} // namespace nile