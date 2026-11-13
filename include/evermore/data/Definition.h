#pragma once

#include <string>

#include <evermore/core/StrongId.h>
#include <evermore/data/UnitStats.h>

struct UnitDefinition
{
    UnitDefinitionId id_;
    std::string name_;
    UnitStats stats_;
};