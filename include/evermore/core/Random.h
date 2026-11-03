#pragma once

#include <random>
#include <cstdint>

struct Random
{
    explicit Random(std::uint32_t seed);

    int NextInt(int min, int max);

private:
    std::mt19937 engine_;
};