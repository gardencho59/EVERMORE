#include <stdexcept>
#include <utility>

#include <evermore/combat/CombatState.h>

CombatState::CombatState(std::vector<CombatUnit> units)
    : units_(std::move(units)), 
      phase_(CombatPhase::Initializing)
{
    for (std::size_t i = 0; i < units_.size(); ++i)
    {
        for (std::size_t j = i + 1; j < units_.size(); ++j)
        {
            if (units_[i].GetId() == units_[j].GetId())
            {
                throw std::invalid_argument("Duplicate combat unit ID");
            }
        }
    }
}

CombatUnit* CombatState::FindUnit(CombatUnitId id)
{
    for (CombatUnit& unit : units_)
    {
        if (unit.GetId() == id)
        {
            return &unit;
        }
    }

    return nullptr;
}

const CombatUnit* CombatState::FindUnit(CombatUnitId id) const
{
    for (const CombatUnit& unit : units_)
    {
        if (unit.GetId() == id)
        {
            return &unit;
        }
    }

    return nullptr;
}

std::span<const CombatUnit> CombatState::GetAllUnits() const
{
    return std::span<const CombatUnit>{units_};
}

std::vector<CombatUnitId> CombatState::GetUnitsByTeam(TeamId team) const
{
    std::vector<CombatUnitId> teamUnitIds;
    
    for (const CombatUnit& unit : units_)
    {
        if (unit.GetTeam() == team)
        {
            teamUnitIds.push_back(unit.GetId());
        }
    }

    return teamUnitIds;
}

CombatPhase CombatState::GetPhase() const noexcept
{
    return phase_;
}
