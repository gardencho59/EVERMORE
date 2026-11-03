#include <evermore/data/GameDatabase.h>

GameDatabase::GameDatabase()
{
    // load static data
    // validate data
    // validata between definition

}


const UnitDefinition& GameDatabase::GetUnitDefinition(UnitDefinitionId id) const
{
    return unitDefinitionData_.at(id);
}

bool GameDatabase::RegisterUnitDefinition(const UnitDefinition& definition)
{
    if (!definition.id_.IsValid())
    {
        return false;
    }

    auto result = unitDefinitionData_.emplace(definition.id_, definition);
    return result.second;
}