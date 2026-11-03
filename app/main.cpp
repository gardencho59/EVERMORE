#include <iostream>
#include <string>

#include <evermore/data/Definition.h>
#include <evermore/data/SampleDefinition.h>
#include <evermore/data/GameDatabase.h>

int main()
{
    GameDatabase database;

    if (!LoadSampleDefinitions(database))
    {
        std::cerr << "Fail to load sample data\n";
        return 1;
    }

    const auto& knight = database.GetUnitDefinition(UnitDefinitionId(10));


    std::cout << "Definition ID: "
              << knight.id_.Value() << "\n";
    std::cout << "Name : "
              << knight.name_ << "\n";
    std::cout << "STR " << knight.stats_.baseStats_.Get(StatType::Strength)
            << " INT " << knight.stats_.baseStats_.Get(StatType::Intelligence)
            << " VIT " << knight.stats_.baseStats_.Get(StatType::Vitality)
            << " TEC " << knight.stats_.baseStats_.Get(StatType::Technique)
            << " AGI " << knight.stats_.baseStats_.Get(StatType::Agility)
            << '\n';

    return 0;
}