#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include <evermore/combat/CombatUnit.h>
#include <evermore/combat/StatCalculator.h>

CombatUnit::CombatUnit(CombatUnitId unitId, TeamId teamId, UnitStats stats, UnitResources resources)
    : id_(unitId), team_(teamId), stats_(stats), resources_(resources)
{   
    if (!id_.IsValid())
    {
        throw std::invalid_argument("Combat Unit ID is not valid");
    }
    
    if (!team_.IsValid())
    {
        throw std::invalid_argument("Team ID is not valid");
    }

    if (resources_.currentHp < 0)
    {
        throw std::invalid_argument("Current HP must be between 0 and max HP");
    }

    const int level = stats_.level;

    if (level < 0)
    {
        throw std::invalid_argument("Level must be greater than 0");
    }

    const double strength = stats.baseStats_.Get(StatType::Strength);
    const double intelligence = stats.baseStats_.Get(StatType::Intelligence);
    const double vitality = stats.baseStats_.Get(StatType::Vitality);
    const double technique = stats.baseStats_.Get(StatType::Technique);
    const double agility = stats.baseStats_.Get(StatType::Agility);

    if (strength < 0 || intelligence < 0 || vitality < 0 || technique < 0 || agility < 0)
    {
        throw std::invalid_argument("Stats cannot be negative");
    }

    const int maxHp = GetMaxHp();

    if (resources_.currentHp > maxHp)
    {
        throw std::invalid_argument("Current HP must be between 0 and max HP");
    }

    if (maxHp <= 0)
    {
        throw std::invalid_argument("Max HP is not valid");
    }

    if (resources_.maxAp < 0)
    {
        throw std::invalid_argument("Max AP is not valid");
    }

    if (resources_.currentAp < 0 || resources_.currentAp > resources_.maxAp)
    {
        throw std::invalid_argument("Current AP must be between 0 and max AP");
    }

    if (resources_.currentShield < 0)
    {
        throw std::invalid_argument("Shield cannot be negative");
    }
}

CombatUnitId CombatUnit::GetId() const noexcept
{
    return id_;
}

TeamId CombatUnit::GetTeam() const noexcept
{
    return team_;
}

int CombatUnit::GetHp() const noexcept
{
    return resources_.currentHp;
}

int CombatUnit::GetMaxHp() const
{
    return GetCombatStats().maxHp;
}

int CombatUnit::GetAp() const noexcept
{
    return resources_.currentAp;
}

int CombatUnit::GetMaxAp() const noexcept
{
    return resources_.maxAp;
}

bool CombatUnit::IsIncapacitated() const noexcept
{
    return GetHp() == 0;
}
   
bool CombatUnit::CanAct() const noexcept
{
    // extra effect after add debuff
    return !IsIncapacitated();
}

int CombatUnit::ApplyHpDelta(int delta)
{
    int oldHp = GetHp();

    const std::int64_t requestedHp = static_cast<std::int64_t>(oldHp) + delta;
    
    const int newHp = static_cast<int>(
        std::clamp<std::int64_t>(requestedHp, 0, GetMaxHp())
    );
    
    resources_.currentHp = newHp;

    return newHp - oldHp;
}

int CombatUnit::ApplyApDelta(int delta)
{
    int oldAp = GetAp();
    
    const std::int64_t requestedAp = static_cast<std::int64_t>(oldAp) + delta;
    
    const int newAp = static_cast<int>(
        std::clamp<std::int64_t>(requestedAp, 0, GetMaxAp())
    );
    
    resources_.currentAp = newAp;

    return newAp - oldAp;
}

CombatStats CombatUnit::GetCombatStats() const
{
    return CalculateCombatStats(stats_);
}

int CombatUnit::GetShield() const noexcept
{
    return resources_.currentShield;
}

int CombatUnit::ApplyShieldDelta(int delta)
{
    int oldShield = GetShield();
    
    const std::int64_t requestedShield = static_cast<std::int64_t>(oldShield) + delta;
    
    const int newShield = static_cast<int>(
        std::clamp<std::int64_t>(requestedShield, 0, std::numeric_limits<int>::max())
    );
    
    resources_.currentShield = newShield;

    return newShield - oldShield;
}

void CombatUnit::ClearShield() noexcept
{
    resources_.currentShield = 0;
}