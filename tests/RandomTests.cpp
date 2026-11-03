#include <cassert>
#include <stdexcept>

#include <evermore/core/Random.h>

void TestRange()
{
    Random random(12345);

    const int min = 0;
    const int max = 10;

    for (int index = 0; index < 1000; ++index)
    {
        const int value = random.NextInt(min, max);
        assert(value >= min && value <= max);
    }
}

void TestSameSeed()
{
    Random random(12345);
    Random random_(12345);

    for (int index = 0; index < 100; ++index)
    {
        const int first = random.NextInt(0, 10);
        const int second = random_.NextInt(0, 10);

        assert(first == second);
    }
}

void TestEqualBounds()
{
    Random random(12345);

    const int value = random.NextInt(10, 10);
    assert(value == 10);
}

void TestInvalidRange()
{
    Random random(12345);
    
    bool caught = false;

    try
    {
        random.NextInt(12, 10);
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }

    assert(caught);
}

int main()
{
    TestRange();
    TestSameSeed();
    TestEqualBounds();
    TestInvalidRange();
}