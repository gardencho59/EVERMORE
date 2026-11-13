#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>

template <typename Tag>
struct StrongId
{
    using value_type = std::uint32_t;
    
    constexpr explicit StrongId(value_type value) noexcept 
        : value_(value) 
    {
    }

    constexpr value_type Value() const noexcept 
    { 
        return value_; 
    }

    constexpr auto operator<=>(const StrongId&) const = default;

    constexpr bool IsValid() const noexcept
    {
        return value_ != 0;
    }

private:
    value_type value_;
};

namespace std
{
    template <typename Tag>
    struct hash<StrongId<Tag>>
    {
        std::size_t operator()(const StrongId<Tag>& id) const noexcept
        {
            return hash<typename StrongId<Tag>::value_type>{}(id.Value());
        }
    };
}

struct UnitDefinitionTag {};
struct CombatUnitTag {};
struct TeamTag {};

using UnitDefinitionId = StrongId<UnitDefinitionTag>;
using CombatUnitId = StrongId<CombatUnitTag>;
using TeamId = StrongId<TeamTag>;