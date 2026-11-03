#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <array>

#include <evermore/core/StrongId.h>

enum class StatType : std::uint8_t
{
    Strength,
    Intelligence,
    Vitality,
    Technique,
    Agility,
    Count
};

constexpr std::size_t kStatCount =
    static_cast<std::size_t>(StatType::Count);

struct StatBlock
{
    std::array<int, kStatCount> values_{};

    int Get(StatType type) const
    {
        return values_.at(static_cast<std::size_t>(type));
    }

    void Set(StatType type, int value)
    {
        values_.at(static_cast<std::size_t>(type)) = value;
    }
};

struct UnitStats
{
    StatBlock baseStats_;
};

struct UnitDefinition
{
    UnitDefinitionId id_;
    std::string name_;
    UnitStats stats_;
};