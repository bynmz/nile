#include "breakout.hpp"
#include <unistd.h>

namespace nile
{
    
Breakout::Breakout() {}

Breakout::~Breakout() {}

void Breakout::start()
{
    loadGameObjects();
    loop();
}

void Breakout::loop()
{
    RenderSystem2D renderSystem2D{
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

        if (auto commandBuffer = nileRenderer.beginFrame())
        {
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

            //gameObjectManager.updateBuffer(frameIndex);
            // render
            nileRenderer.beginSwapChainRenderPass(commandBuffer);

            renderSystem2D.renderGameObjects(frameInfo);
            playerController.moveInPlaneXY(
                nileWindow.getGLFWwindow(), 
                frameInfo.frameTime, 
                gameObjectManager.gameObjects.at(player->getId())
            );
            // Player presses the space bar
            action(
                nileWindow.getGLFWwindow(), 
                gameObjectManager.gameObjects.at(ballobj->getId())
            );
            updateBallPos(0.05f, WIDTH / 800.0f);

            // Check for collisions
            DoCollisions(frameInfo);

            nileRenderer.endSwapChainRenderPass(commandBuffer);
            nileRenderer.endFrame();
        }
    }
    vkDeviceWaitIdle(nileDevice.device());
}

void Breakout::loadGameLevels()
{
    GameLevel one;
    one.load(
        nileDevice, 
        gameObjectManager, 
        "../resources/breakout/levels/one.lvl", 
        this->WIDTH, this->HEIGHT / 2, 
        true);
    GameLevel two;
    two.load(
        nileDevice, 
        gameObjectManager, 
        "../resources/breakout/levels/two.lvl", 
        this->WIDTH, this->HEIGHT / 2);
    GameLevel three;
    three.load(
        nileDevice, 
        gameObjectManager, 
        "../resources/breakout/levels/three.lvl", 
        this->WIDTH, this->HEIGHT / 2);
    GameLevel four;
    four.load(
        nileDevice, 
        gameObjectManager, 
        "../resources/breakout/levels/four.lvl", 
        this->WIDTH, this->HEIGHT / 2);

    this->Levels.emplace_back(one);
    this->Levels.emplace_back(two);
    this->Levels.emplace_back(three);
    this->Levels.emplace_back(four);

    Level = 0;
}

void Breakout::loadGameObjects()
{    
    
    std::shared_ptr<NileTexture> smileyFace = 
        NileTexture::createTextureFromFile(nileDevice, "../resources/breakout/images/awesomeFace.png");
    ballobj = &gameObjectManager.makeBall();
    ballobj->model = createCircleSprite(nileDevice, 22);
    ballobj->color = glm::vec3(1.0f);
    ballobj->transform2d.translation = {0.0f, .84f, 0.f};
    ballobj->diffuseMap = smileyFace;

    
    std::shared_ptr<NileTexture> paddle = 
        NileTexture::createTextureFromFile(nileDevice, "../resources/breakout/images/paddle.png");
    player = &gameObjectManager.createGameObject();
    player->model = createRectangleSprite(nileDevice, 100.0f / 50, 20.0f / 10);
    player->transform2d.scale ={.2f, .05f};
    player->transform2d.translation = {0.0f, .94f, 0.f};
    player->color = glm::vec3(1.0f);
    player->diffuseMap = paddle; 
    
    
    loadGameLevels();
}

std::unique_ptr<NileModel> Breakout::createCircleSprite(NileDevice& device, unsigned int numSides)
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

std::unique_ptr<NileModel> Breakout::createOctopusSprite(NileDevice& device) {
    std::vector<NileModel::Vertex> vertices{
        {{0.0f, 0.2f, 0.0f}},  // Head center
        {{-0.2f, 0.0f, 0.0f}}, // Head left
        {{0.2f, 0.0f, 0.0f}},  // Head right
        // Tentacles
        {{-0.3f, -0.2f, 0.0f}},
        {{-0.15f, -0.35f, 0.0f}},
        {{0.0f, -0.5f, 0.0f}},
        {{0.15f, -0.35f, 0.0f}},
        {{0.3f, -0.2f, 0.0f}}
    };

    std::vector<uint32_t> indices{
        0, 1, 2, // Head triangle
        1, 3, 4, // Left tentacle
        1, 4, 5, // Inner left tentacle
        2, 6, 7, // Right tentacle
        2, 5, 6  // Inner right tentacle
    };

    // Create the NileModel using the provided vertices and indices
    NileModel::Builder spriteBuilder{};
    spriteBuilder.vertices = vertices;
    spriteBuilder.indices = indices;
    return std::make_unique<NileModel>(device, spriteBuilder);
}

std::unique_ptr<NileModel> Breakout::createRectangleSprite(NileDevice& device, float width, float height)
{

    // Number of segments for rounded corners
    const int numSegments = 5;
    const float cornerRadius = 0.5f;
    
    std::vector<NileModel::Vertex> vertices;

    // Function to add corner vertices
    auto addCornerVertices = [&](float centerX, float centerY, float startAngle) {
        for (int i = 0; i <= numSegments; ++i) {
            float angle = startAngle + i * M_PI_2 / numSegments;
            vertices.push_back({{centerX + cornerRadius * cos(angle), centerY + cornerRadius * sin(angle), 0.0f}});
        }
    };

    // Add vertices for each corner
    addCornerVertices(-width / 2.0f + cornerRadius, -height / 2.0f + cornerRadius, M_PI);
    addCornerVertices(width / 2.0f - cornerRadius, -height / 2.0f + cornerRadius, -M_PI_2);
    addCornerVertices(width / 2.0f - cornerRadius, height / 2.0f - cornerRadius, 0.0f);
    addCornerVertices(-width / 2.0f + cornerRadius, height / 2.0f - cornerRadius, M_PI_2);

    // Add center vertices for straight edges
    vertices.push_back({{-width / 2.0f + cornerRadius, -height / 2.0f, 0.0f}});
    vertices.push_back({{width / 2.0f - cornerRadius, -height / 2.0f, 0.0f}});
    vertices.push_back({{width / 2.0f, -height / 2.0f + cornerRadius, 0.0f}});
    vertices.push_back({{width / 2.0f, height / 2.0f - cornerRadius, 0.0f}});
    vertices.push_back({{width / 2.0f - cornerRadius, height / 2.0f, 0.0f}});
    vertices.push_back({{-width / 2.0f + cornerRadius, height / 2.0f, 0.0f}});
    vertices.push_back({{-width / 2.0f, height / 2.0f - cornerRadius, 0.0f}});
    vertices.push_back({{-width / 2.0f, -height / 2.0f + cornerRadius, 0.0f}});

    // Simplified indices, you need to carefully construct these
    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < vertices.size() - 2; ++i) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    // Ensure vertices and indices are not empty
    if (vertices.empty() || indices.empty()) {
        throw std::runtime_error("Vertex or index buffer is empty!");
    }

    // Create the NileModel using the provided vertices and indices
    NileModel::Builder spriteBuilder{};
    spriteBuilder.vertices = vertices;
    spriteBuilder.indices = indices;
    return std::make_unique<NileModel>(device, spriteBuilder);
}

void Breakout::updateBallPos(float dt, unsigned int window_width)
{
    // if not stuck to player board
    if (!ballobj->ball->stuck)
    {
        //move the ball
        ballobj->transform2d.translation += glm::vec3(ballobj->rigidBody2d.velocity * dt, 0.f);
        // check if outside window bounds; if so, reverse velocity and restore at correct position
        if (ballobj->transform2d.translation.x <= -1.0f)
        {
            ballobj->rigidBody2d.velocity.x = -ballobj->rigidBody2d.velocity.x;
            ballobj->transform2d.translation.x = -1.0f;
        }
        else if (ballobj->transform2d.translation.x + ballobj->transform2d.scale.x >= window_width)
        {
            ballobj->rigidBody2d.velocity.x = -ballobj->rigidBody2d.velocity.x;
            ballobj->transform2d.translation.x = window_width - ballobj->transform2d.scale.x;
        }
        if (ballobj->transform2d.translation.y <= -1.0f)
        {
            ballobj->rigidBody2d.velocity.y = -ballobj->rigidBody2d.velocity.y;
            ballobj->transform2d.translation.y = -1.0f;
        }
        else if (ballobj->transform2d.translation.y + ballobj->transform2d.scale.y >= window_width)
        {
            // Reset
            // ballobj->rigidBody2d.velocity.y = ballobj->rigidBody2d.velocity.y * -1;
            // ballobj->transform2d.translation.y = .7f;
            //ballobj->ball->stuck = !ballobj->ball->stuck;
        }
    } else 
    {
        ballobj->transform2d.translation.x = player->transform2d.translation.x;
    }
}

void Breakout::action(
    GLFWwindow* window, NileGameObject &gameObject)
{
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        gameObject.ball->stuck = false;
    }
}

// bool Breakout::checkCollision(NileGameObject &one, NileGameObject &two) //  AABB - AABB collision
// {
//     // collision x-axis?
//     bool collisionX = one.transform2d.translation.x + one.transform2d.scale.x >= two.transform2d.translation.x &&
//         two.transform2d.translation.x + two.transform2d.scale.x >= one.transform2d.translation.x;
//     // collision y-axis?
//     bool collisionY = one.transform2d.translation.y + one.transform2d.scale.y >= two.transform2d.translation.y &&
//         two.transform2d.translation.y + two.transform2d.scale.y >= one.transform2d.translation.y;
//     // collision only if on both axes
//     return collisionX && collisionY;
// }

Breakout::Collision Breakout::checkCollision(NileGameObject &one, NileGameObject &two) //  AABB - Circle collision
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

void Breakout::DoCollisions(FrameInfo frameInfo)
{
    for (NileGameObject::id_t id : Levels[this->Level].bricks)
    {
        auto & obj = frameInfo.gameObjects.at(id);
        if (!obj.Destroyed)
        {
            Collision collision = checkCollision(*ballobj, obj);
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
                    ballobj->rigidBody2d.velocity.x = -ballobj->rigidBody2d.velocity.x;
                    // relocate
                    float penetration = ballobj->ball->radius - std::abs(diff_vector.x);
                    if (dir == LEFT)
                        // move ball to right
                        ballobj->transform2d.translation.x += penetration;
                    else
                        // move ball to left;
                        ballobj->transform2d.translation.x -= penetration;
                }
                else  // vertical collision
                {
                    // reverse vertical velocity
                    ballobj->rigidBody2d.velocity.y = -ballobj->rigidBody2d.velocity.y;
                    // relocate
                    float penetration = ballobj->ball->radius - std::abs(diff_vector.y);
                    if (dir == UP)
                        // move ball back up
                        ballobj->transform2d.translation.y -= penetration;
                    else
                        // move ball back down
                        ballobj->transform2d.translation.y += penetration;
                }
            }
        }
    };
    Collision result = checkCollision(*ballobj, *player);
    if (!ballobj->ball->stuck && std::get<0>(result))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = player->transform2d.translation.x + player->transform2d.scale.x / 2.0f;
        float distance = (ballobj->transform2d.translation.x + ballobj->ball->radius) - centerBoard;
        float percentage = distance / (player->transform2d.scale.x / 2.0f);
        // then move accordingly
        float strength = 2.f;
        glm::vec2 oldVelocity = ballobj->rigidBody2d.velocity;
        ballobj->rigidBody2d.velocity.x = .05f * percentage * strength; // .04f is the initial ball velocity
        ballobj->rigidBody2d.velocity.y = -ballobj->rigidBody2d.velocity.y;
        ballobj->rigidBody2d.velocity = glm::normalize(ballobj->rigidBody2d.velocity) * glm::length(oldVelocity);
    }
}

Breakout::Direction Breakout::VectorDirection(glm::vec2 target)
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

void GameLevel::load(
        NileDevice& device, 
        NileGameObjectManager& gom, 
        const std::string& file, 
        unsigned int levelWidth, 
        unsigned int levelHeight,
        bool isCurrentLevel)
{
    bricks.clear();
    unsigned int tileCode;
    // GameLevel level;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<unsigned int>> tileData;

    if(fstream.is_open()) {
        while (std::getline(fstream, line)) // Read each line from the level file
        {
            std::istringstream sstream(line);
            std::vector<unsigned int> row;
            while (sstream >> tileCode) // read each word separated by spaces
                row.push_back(tileCode);
            tileData.push_back(row);
        }
        fstream.close();

        if (!tileData.empty())
            this->init(device, gom, tileData, levelWidth, levelHeight, isCurrentLevel);
        else
            std::cerr << "Error: Empty tile data loaded from file: " << file << std::endl;
    } else {
        std::cerr << "Error: Failed to open file: " << file << std::endl;
    }
}

void GameLevel::init(NileDevice& device, 
    NileGameObjectManager& gom,
    const std::vector<std::vector<unsigned int>>& tileData,
    unsigned int levelWidth, unsigned int levelHeight, bool isCurrentLevel)
{
    // calculates dimensions
    unsigned int height = tileData.size();
    unsigned int width = tileData[0].size();
    float unit_width = levelWidth / static_cast<float>(width);
    unit_width = unit_width / 400;
    float unit_height = levelHeight / height;
    unit_height = unit_height / 400;

    std::shared_ptr<NileTexture> blockSolid = 
        NileTexture::createTextureFromFile(device, "../resources/breakout/images/block_solid.png");
    std::shared_ptr<NileTexture> block = 
        NileTexture::createTextureFromFile(device, "../resources/breakout/images/block.png");
    std::shared_ptr<NileTexture> texture =
        NileTexture::createTextureFromFile(device, "../resources/breakout/images/background.jpg");

    

    NileModel::Builder meshBuilder{};
    meshBuilder.vertices = {
        // vertices (position, color)
        {{-0.5f, -0.5f, 0.0f}, glm::vec3(1.f)}, // bottom left
        {{0.5f, -0.5f, 0.0f}, glm::vec3(1.f)},  // bottom right
        {{0.5f, 0.5f, 0.0f}, glm::vec3(1.f)},   // top right
        {{-0.5f, 0.5f, 0.0f}, glm::vec3(1.f)}   // top left
    };
     // indices to form two triangles (bottom left, bottom right, top right) and (bottom left, top right, top left)
    meshBuilder.indices = {0, 1, 2, 0, 2, 3};
    auto square = std::make_shared<NileModel>(device, meshBuilder);

    // background
    glm::vec2 size(width/4, height/4);
    auto& bg = gom.createGameObject();
    bg.model = square;
    bg.color = glm::vec3(1.0f);
    bg.diffuseMap = texture;
    bg.transform2d.translation = {0, 0, 0.01f}; 
    bg.transform2d.scale = size;
    bg.setIsHidden(!isCurrentLevel);

    // Initialize level tiles based on tileData
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            // check block type from level data (2D level array)
            if (tileData[y][x] == 1) // solid
            {
                glm::vec3 pos(unit_width * x - .93f, unit_height * y - .95f, 0.f);
                glm::vec2 size(unit_width, unit_height);
                
                auto& obj = gom.createGameObject();
                obj.diffuseMap = blockSolid;
                obj.transform2d.translation = pos;
                obj.transform2d.scale = size;
                obj.color = glm::vec3(0.8f, 0.8f, 0.7f);
                obj.isSolid = true;
                obj.model = square;
                obj.setIsHidden(!isCurrentLevel);
                bricks.push_back(obj.getId());
            }
            else if (tileData[y][x] > 1)
            {
                glm::vec3 color = glm::vec3(1.0f);  // original: white
                if (tileData[y][x] == 2)
                    color = glm::vec3(0.2f, 0.6f, 1.0f);
                else if (tileData[y][x] == 3)
                    color = glm::vec3(0.0f, 0.7f, 0.0f);
                else if (tileData[y][x] == 4)
                    color = glm::vec3(0.8f, 0.8f, 0.4f);
                else if (tileData[y][x] == 5)
                    color = glm::vec3(1.0f, 0.5f, 0.0f);
                
                glm::vec3 pos(unit_width * x - .93f, unit_height * y - .95f, 0.f);
                glm::vec2 size(unit_width, unit_height);

                auto& obj = gom.createGameObject();
                obj.diffuseMap = block;
                obj.transform2d.translation = pos;
                obj.transform2d.scale = size;
                obj.color = color;
                obj.model = square;
                obj.setIsHidden(!isCurrentLevel);
                bricks.push_back(obj.getId());
            }
        }
    }
}

} // namespace nile