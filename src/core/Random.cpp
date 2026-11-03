
#include <stdexcept>

#include <evermore/core/Random.h>

Random::Random(std::uint32_t seed)
    : engine_(seed) 
{
}

int Random::NextInt(int min, int max)
{
    if (min > max)
    {
        throw std::invalid_argument("min should not be greater than max");
    }

    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(engine_);
}