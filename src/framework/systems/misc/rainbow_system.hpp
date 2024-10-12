#pragma once

#include "framework/core/nile_game_object.hpp"
#include "framework/core/nile_frame_info.hpp"

// std
#include <memory>
#include <random>
#include <vector>

namespace nile
{
class RainbowSystem
{
private:
    std::random_device rd;
    std::mt19937 mRng{rd()};

    std::vector<glm::vec3> mColors;
    float mFlickerRate;
    float mElapsedTime;

public:
    RainbowSystem(float flickerRate) : mFlickerRate(flickerRate) {
        // Initialize colors
        mColors = {
            {.8f, .1f, .1f},
            {.1f, .8f, .1f},
            {.1f, .1f, .8f},
            {.8f, .8f, .1f},
            {.8f, .1f, .8f},
            {.1f, .8f, .8f},
        };
        mElapsedTime = mFlickerRate;
    }

    // Randomly select a color for each game object every mFlickerRate seconds
    void update(float dt, FrameInfo2d& frameInfo) {
        mElapsedTime -= dt;
        if (mElapsedTime < 0.f) {
            mElapsedTime += mFlickerRate;
            std::uniform_int_distribution<int> randInt{0, static_cast<int>(mColors.size()) - 1};
            for (auto &kv : frameInfo.gameObjects) {
                auto& obj = kv.second;
                if (obj.sprite == nullptr) continue;

                int randValue = randInt(mRng);
                obj.color = mColors[randValue];
            }
        }
    }
};

} // namespace nile
