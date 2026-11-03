
#include <evermore/data/SampleDefinition.h>
#include <evermore/data/Definition.h>
#include <evermore/data/GameDatabase.h>

bool LoadSampleDefinitions(GameDatabase& database)
{
    UnitDefinitionId sampleId{10};
    std::string sampleName = "Sample Knight";
    StatBlock sampleStats;
    sampleStats.Set(StatType::Strength, 6);
    sampleStats.Set(StatType::Intelligence, 12);
    sampleStats.Set(StatType::Vitality, 9);
    sampleStats.Set(StatType::Technique, 8);
    sampleStats.Set(StatType::Agility, 10);

    UnitDefinition sampleKnight(
        sampleId, 
        sampleName, 
        UnitStats{sampleStats}
    );

    return database.RegisterUnitDefinition(sampleKnight);
}