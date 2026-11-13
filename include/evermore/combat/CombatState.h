#pragma once

#include <span>
#include <cstdint>
#include <vector>

#include <evermore/combat/CombatUnit.h>

enum class CombatPhase : std::uint8_t
{
    Initializing,
    WaitForTurn,
    WaitingForAction,
    ResolvingAction,
    Finished
};

class CombatState
{
    public:
        explicit CombatState(std::vector<CombatUnit> units);

        CombatUnit* FindUnit(CombatUnitId id);
        const CombatUnit* FindUnit(CombatUnitId id) const;

        std::span<const CombatUnit> GetAllUnits() const;
        std::vector<CombatUnitId> GetUnitsByTeam(TeamId team) const;
        CombatPhase GetPhase() const noexcept;

    private:
        std::vector<CombatUnit> units_;
        CombatPhase phase_;
};