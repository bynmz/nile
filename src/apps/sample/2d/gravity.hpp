#include "app_2d.hpp"

#include "framework/systems/physics/gravity_system.hpp"
#include "framework/systems/physics/vec2_field_system.hpp"

namespace nile
{
    
class Gravity : public App2D
{
private:   
    std::unique_ptr<NileModel>  createCircleSprite(NileDevice& device, unsigned int numSides);
    std::unique_ptr<NileModel>  createSquareSprite(NileDevice& device, glm::vec3 offset);

    std::vector<NileGameObject*> vectorField{};
    std::vector<NileGameObject*> physObjects{};

    void loadGameObjects() override;
public:
    Gravity();
    ~Gravity() override;

    void loop() override;
    void start() override;
};

Gravity::Gravity(){}

Gravity::~Gravity(){}

std::unique_ptr<NileModel> Gravity::createSquareSprite(NileDevice& device, glm::vec3 offset)
{
    std::vector<NileModel::Vertex> vertices = {
        {{-0.5f, -0.5f, 0.f}},
        {{0.5f, -0.5f, 0.f}},
        {{0.5f, 0.5f, 0.f}},
        {{-0.5f, 0.5f, 0.f}},
    };
    for (auto& v : vertices) {
        v.position += offset;
    }
    NileModel::Builder spriteBuilder{};
    spriteBuilder.vertices = vertices;
    spriteBuilder.indices.push_back(0);
    spriteBuilder.indices.push_back(1);
    spriteBuilder.indices.push_back(2);
    spriteBuilder.indices.push_back(0);
    spriteBuilder.indices.push_back(2);
    spriteBuilder.indices.push_back(3);
    return std::make_unique<NileModel>(device, spriteBuilder);
}
std::unique_ptr<NileModel> Gravity::createCircleSprite(NileDevice& device, unsigned int numSides)
{
    std::vector<NileModel::Vertex> uniqueVertices{};
    for (int i = 0; i < numSides; i++) {
        float angle = i * glm::two_pi<float>() / numSides;
        uniqueVertices.push_back({{glm::cos(angle), glm::sin(angle), 0}});
    }
    uniqueVertices.push_back({});   // adds center vertex at 0, 0

    std::vector<NileModel::Vertex> vertices{};
    std::vector<uint32_t> indices{};
    for (int i = 0; i < numSides; i++) {
        vertices.push_back(uniqueVertices[i]);
        vertices.push_back(uniqueVertices[(i + 1) % numSides]);
        vertices.push_back(uniqueVertices[numSides]);

        indices.push_back(i * 3);
        indices.push_back(i * 3 + 1);
        indices.push_back(i * 3 + 2);
    }

    NileModel::Builder spriteBuilder{};
    spriteBuilder.vertices = vertices;
    spriteBuilder.indices = indices;

    return std::make_unique<NileModel>(device, spriteBuilder);
}

void Gravity::loadGameObjects() {
    // Create some sprites
    std::shared_ptr<NileModel> square = createSquareSprite(
        nileDevice,
        {0.5f, 1.f, 0.f});   // offset sprite by .5 so rotation occurs at edge rather than center of square
    
    std::shared_ptr<NileModel> circle = createCircleSprite(nileDevice, 22);

    // create physics objects
    auto& blue = gameObjectManager.createGameObject();
    blue.transform2d.scale = glm::vec2{.05f};
    blue.transform2d.translation = {.5f, .5f};
    blue.color = {0.f, 0.f, 1.f};
    blue.rigidBody2d.velocity = {-.5f, .0f};
    blue.model = circle;
    physObjects.push_back(&blue);

    auto& red = gameObjectManager.createGameObject();
    red.transform2d.scale = glm::vec2{.05f};
    red.transform2d.translation = {-.45f, -.25f};
    red.color = {1.f, 0.f, 0.f};
    red.rigidBody2d.velocity = {.5f, .0f};
    red.model = circle;
    physObjects.push_back(&red);

    // create vector field
    int gridCount = 30;
    for (int i = 0; i < gridCount; i++) {
        for (int j = 0; j < gridCount; j++) {
            auto& vf = gameObjectManager.createGameObject();
            vf.transform2d.scale = glm::vec2(0.005f);
            vf.transform2d.translation = {
                -1.0f + (i + 0.5f) * 2.0f / gridCount,
                -1.0f + (j + 0.5f) * 2.0f / gridCount,
            };
            vf.color = {1.f, 1.f, 1.f};
            vf.model = square;
            vf.isVectorField = true;
            vectorField.push_back(&vf);
        }

    }
}

void Gravity::loop() {
    GravityPhysicsSystem gravitySystem{0.81f};
    Vec2FieldSystem vecFieldSystem{};
    RenderSystem2D rendersys{
        nileDevice,
        nileRenderer.getSwapChainRenderPass(),
        globalSetLayout->getDescriptorSetLayout()
    };

    auto currentTime = std::chrono::high_resolution_clock::now();

    while (!nileWindow.shouldClose())
    {
        glfwPollEvents();
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =
            std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        if (auto commandBuffer = nileRenderer.beginFrame()) {
            
            int frameIndex = nileRenderer.getFrameIndex();
            framePools[frameIndex]->resetPool();
            FrameInfo frameInfo{
                frameIndex,
                frameTime,
                commandBuffer,
                camera,
                globalDescriptorSets[frameIndex],
                *framePools[frameIndex],
                gameObjectManager.gameObjects};

            // update systems
            gravitySystem.update(physObjects, 1.f / 60, 5);
            vecFieldSystem.update(gravitySystem, gameObjectManager.gameObjects, vectorField);

            // render system
            nileRenderer.beginSwapChainRenderPass(commandBuffer);
            rendersys.renderGameObjects(frameInfo);
            nileRenderer.endSwapChainRenderPass(commandBuffer);
            nileRenderer.endFrame();
        }
    }
    vkDeviceWaitIdle(nileDevice.device());
}

void Gravity::start(){
    loadGameObjects();
    loop();
}

} // namespace nile
