#include "heights_generator_system.hpp"

#include <iostream>

namespace nile
{
HeightsGenerator::HeightsGenerator()
{
    std::random_device rd;
    seed = rd();
    gen.seed(seed);
}

HeightsGenerator::HeightsGenerator(int gridX, int gridZ, int vertexCount, int seed)
{
    this->seed = seed;
    gen.seed(seed);
    xOffset = gridX * (vertexCount-1);
    zOffset = gridZ * (vertexCount-1);
}

HeightsGenerator::~HeightsGenerator()
{
}

float HeightsGenerator::generateHeight(int x, int z)
{
    float total = 0;
    float d = (float)std::pow(2, OCTAVES - 1);
    for (int i = 0; i < OCTAVES; i++)
    {
        float freq = (float)(std::pow(2, i) / d);
        float amp = (float)(std::pow(ROUGHNESS, i) * AMPLITUDE);
        total += getInterpolatedNoise(x*freq, z) * amp;
    }
    return total;
    
}

float HeightsGenerator::getInterpolatedNoise(float x, float z)
{
    int intX = (int) x;
    int intZ = (int) z;
    float fracX = x - intX;
    float fracZ = z - intZ;

    float v1 = getSmoothNoise(intX, intZ);
    float v2 = getSmoothNoise(intX + 1, intZ);
    float v3 = getSmoothNoise(intX, intZ + 1);
    float v4 = getSmoothNoise(intX + 1, intZ + 1);

    float i1 = interpolate(v1, v2, fracX);
    float i2 = interpolate(v3, v4, fracX);
    return interpolate(i1, i2, fracZ);
}

float HeightsGenerator::interpolate(float a, float b, float blend)
{
    double theta = blend * M_PI;
    float f = (float)(1.f - std::cos(theta)) * 0.5f;
    return a * (1.f - f) + b * f;
}

float HeightsGenerator::getSmoothNoise(int x, int z)
{
    float corners = (getNoise(x - 1, z - 1) + getNoise(x + 1, z - 1) + getNoise(x - 1, z + 1)
                    + getNoise(x + 1, z + 1)) / 16.f;
    float sides = (getNoise(x-1, z)+getNoise(x+1, z)+getNoise(x, z-1)
                    +getNoise(x, z+1))/8.f;
    float center = getNoise(x, z) / 4.f;
    return corners + sides + center;
}

float HeightsGenerator::getNoise(int x, int z)
{
    gen.seed(x * X_FACTOR + z * Z_FACTOR + seed);
    return distrib(gen) * 2.0f - 1.0f;
}

}
