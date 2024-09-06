#pragma once
#include <random>

class RandomNumberGenerator {
public:
    RandomNumberGenerator() : dev(), rng(dev()), dist(0, 1) {
    }

    float operator()() {
        return dist(rng);
    }

private:
    std::random_device dev;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
};
