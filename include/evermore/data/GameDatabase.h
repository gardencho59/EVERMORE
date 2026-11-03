#pragma once

#include <unordered_map>

#include <evermore/data/Definition.h>

class GameDatabase
{

public:
    explicit GameDatabase();

    const UnitDefinition& GetUnitDefinition(UnitDefinitionId id) const;

    bool RegisterUnitDefinition(const UnitDefinition& definition);

private:
    std::unordered_map<UnitDefinitionId, UnitDefinition> unitDefinitionData_;
};