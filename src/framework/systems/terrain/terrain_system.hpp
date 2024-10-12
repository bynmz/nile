#pragma once

#include "heights_generator_system.hpp"
#include "../material/material_pack_system.hpp"
#include "../../core/nile_model.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <string>

namespace nile 
{
class ProceduralTerrain
{
private:
    const float SIZE = 6;
    const float MAX_HEIGHT = 40;
    const float MAX_PIXEL_COLOR = 256 * 256 * 256;
    std::shared_ptr<HeightsGenerator> generator;

    const int VERTEX_COUNT = 6;
    std::random_device rd;
    const unsigned int SEED{rd() % 1000000000};
    std::mt19937 gen;
    std::uniform_real_distribution<float> distrib{-1.0f, 1.0f};

    float x;
    float z;
    std::shared_ptr<MaterialPack> textures;
    
    std::vector<std::vector<float>> heights;
    NileDevice &device;
    
public:
    ProceduralTerrain(
        NileDevice &device, 
        int gridX, 
        int gridZ, 
        std::shared_ptr<MaterialPack> textures, 
        std::string heightMap);
    ~ProceduralTerrain();

    std::shared_ptr<NileModel> mesh;
    
    float getX() { return x; }
    float getZ() { return z; }
    const float getSize() { return SIZE; }
    std::shared_ptr<NileModel> getMesh() { return mesh; }
    float getHeightOfTerrain(float worldX, float worldZ);
    std::shared_ptr<NileModel> generateTerrain(std::shared_ptr<MaterialPack> textures, std::string path);
    glm::vec3 calculateNormal(int x, int z, std::shared_ptr<HeightsGenerator> generator);
    float getHeight(int x, int z, std::shared_ptr<HeightsGenerator> generator);
};


}