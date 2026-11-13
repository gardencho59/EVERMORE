#pragma once

#include <evermore/core/StrongId.h>
#include <evermore/data/UnitStats.h>
#include <evermore/combat/CombatStats.h>

struct UnitResources
{
    int currentHp = 1;
    int currentAp = 1;
    int maxAp = 7;
    int currentShield = 0;
};

class CombatUnit
{
public:
    CombatUnit(CombatUnitId unitId, TeamId teamId, UnitStats stats, UnitResources resources);

    CombatUnitId GetId() const noexcept;
    TeamId GetTeam() const noexcept;

    int GetHp() const noexcept;
    int GetMaxHp() const;

    int GetAp() const noexcept;
    int GetMaxAp() const noexcept;

    int GetShield() const noexcept;

    bool IsIncapacitated() const noexcept;
    bool CanAct() const noexcept;

    int ApplyHpDelta(int delta);
    int ApplyApDelta(int delta);
    int ApplyShieldDelta(int delta);

    void ClearShield() noexcept;

    CombatStats GetCombatStats() const;

private:
    CombatUnitId id_;
    TeamId team_;

    UnitStats stats_;

    UnitResources resources_;

    /*
    std::vector<SkillId> equippedSkills_;
    std::vector<ActiveStatus> statuses_;
    */
};