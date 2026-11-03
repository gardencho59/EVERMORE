#include <cassert>
#include <stdexcept>

#include <evermore/data/GameDatabase.h>
#include <evermore/data/SampleDefinition.h>

void TestLoadSampleDefinitions()
{
    GameDatabase database;

    const bool loaded = LoadSampleDefinitions(database);
    assert(loaded);

    const auto& knight = database.GetUnitDefinition(UnitDefinitionId{10});

    assert(knight.name_ == "Sample Knight");

    const std::array<int, kStatCount> statValues{6, 12, 9, 8, 10};

    for (std::size_t index = 0; index < kStatCount; ++index)
    {
        assert(knight.stats_.baseStats_.values_[index] == statValues[index]);
    }
    
}

void TestDuplicatedId()
{
    GameDatabase database;
    
    const bool loaded = LoadSampleDefinitions(database);
    assert(loaded);

    const bool reload = database.RegisterUnitDefinition(UnitDefinition{UnitDefinitionId{10}, "Invalid", UnitStats{}});
    assert(!reload);

    const auto& knight = database.GetUnitDefinition(UnitDefinitionId{10});
    
    assert(knight.name_ == "Sample Knight");
}

void TestInvalidId()
{
    
    GameDatabase database;

    const bool loaded = database.RegisterUnitDefinition(UnitDefinition{UnitDefinitionId{0}, "Invalid", UnitStats{}});
    assert(!loaded);
}

void TestMissingId()
{
    GameDatabase database;

    bool caught = false;

    try
    {
        database.GetUnitDefinition(UnitDefinitionId{999});
    }
    catch (const std::out_of_range&)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    TestLoadSampleDefinitions();
    TestDuplicatedId();
    TestInvalidId();
    TestMissingId();
}