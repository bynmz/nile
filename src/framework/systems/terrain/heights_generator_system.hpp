#ifndef HEIGHTS_GENERATOR_SYSTEM_HPP
#define HEIGHTS_GENERATOR_SYSTEM_HPP

#include <random>
#include <cmath>

namespace nile
{
class HeightsGenerator
{
private:
    // Magic numbers
    const int X_FACTOR = 49632;
    const int Z_FACTOR = 325176;

    const float AMPLITUDE = 1.f;
    const int OCTAVES = 3;
    const float ROUGHNESS = 0.3f;

    std::mt19937 gen;
    std::uniform_real_distribution<float> distrib{-1.0f, 1.0f};
    int seed;
    int xOffset = 0;
    int zOffset = 0;

    float getInterpolatedNoise(float x, float z);
    float interpolate(float a, float b, float blend);
    float getSmoothNoise(int x, int z);
    float getNoise(int x, int z);

public:
    HeightsGenerator();
    HeightsGenerator(int gridX, int gridZ, int vertexCount, int seed);
    ~HeightsGenerator();

    float generateHeight(int x, int z);
};
}

#endif // HEIGHTS_GENERATOR_SYSTEM_HPP