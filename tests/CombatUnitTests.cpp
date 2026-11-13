#include <cassert>
#include <limits>
#include <stdexcept>

#include <evermore/combat/CombatUnit.h>

UnitStats MakeStats(int vitality = 9)
{
    UnitStats stats{};
    stats.baseStats_.Set(StatType::Vitality, vitality);
    return stats;
}

CombatUnit MakeUnit(UnitResources resources = {20, 1, 7},
                    CombatUnitId id = CombatUnitId{10}, TeamId team = TeamId{1})
{
    return CombatUnit{
        id,
        team,
        MakeStats(),
        resources
    };
}

void TestDamageAndIndependence()
{
    auto unitA = MakeUnit();
    auto unitB = MakeUnit(UnitResources{20, 1, 7}, CombatUnitId{20});

    const int change = unitA.ApplyHpDelta(-30);
    assert(change == -20);
    assert(unitA.GetHp() == 0);
    assert(unitB.GetHp() == 20);
    assert(unitA.GetId() != unitB.GetId());
    assert(unitA.IsIncapacitated());
    assert(!unitA.CanAct());
    assert(unitB.CanAct());
}

void TestHealingLimit()
{
    auto unit = MakeUnit();

    assert(unit.CanAct());

    const int change = unit.ApplyHpDelta(1000);

    assert(unit.GetHp() == 78);
    assert(change == 58);

    const int extraChange = unit.ApplyHpDelta(10);

    assert(unit.GetHp() == 78);
    assert(extraChange == 0);
}

void TestApLimits()
{
    auto unit = MakeUnit(UnitResources{20, 6, 7});

    const int gained = unit.ApplyApDelta(3);

    assert(unit.GetAp() == 7);
    assert(gained == 1);

    const int lost = unit.ApplyApDelta(-10);

    assert(unit.GetAp() == 0);
    assert(lost == -7);
}

void TestZeroDelta()
{
    auto unit = MakeUnit();

    const int hpChange = unit.ApplyHpDelta(0);
    const int apChange = unit.ApplyApDelta(0);

    assert(hpChange == 0);
    assert(apChange == 0);
    assert(unit.GetHp() == 20);
    assert(unit.GetAp() == 1);
}

void TestExtremeDeltas()
{
    auto unit = MakeUnit();

    const int hpGain =
        unit.ApplyHpDelta(std::numeric_limits<int>::max());

    assert(unit.GetHp() == 78);
    assert(hpGain == 58);

    const int hpLoss =
        unit.ApplyHpDelta(std::numeric_limits<int>::min());

    assert(unit.GetHp() == 0);
    assert(hpLoss == -78);

    const int apGain =
        unit.ApplyApDelta(std::numeric_limits<int>::max());

    assert(unit.GetAp() == 7);
    assert(apGain == 6);

    const int apLoss =
        unit.ApplyApDelta(std::numeric_limits<int>::min());

    assert(unit.GetAp() == 0);
    assert(apLoss == -7);
}

void TestNegativeHpRejected()
{
    bool caught = false;

    try
    {
        const auto unit = MakeUnit(UnitResources{-1, 1, 7});
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }

    assert(caught);
}

void TestInvalidResourcesRejected()
{
    const UnitResources invalidValues[] = {
        {79, 1, 7}, {20, -1, 7}, {20, 8, 7}, {20, 0, -1}
    };
    for (const auto& resources : invalidValues)
    {
        bool caught = false;
        try
        {
            const auto unit = MakeUnit(resources);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught);
    }
}

void TestInvalidIdsRejected()
{
    const CombatUnitId unitIds[] = {CombatUnitId{0}, CombatUnitId{10}};
    const TeamId teamIds[] = {TeamId{1}, TeamId{0}};
    for (int index = 0; index < 2; ++index)
    {
        bool caught = false;
        try
        {
            const CombatUnit unit{
                unitIds[index], teamIds[index],
                MakeStats(), UnitResources{20, 1, 7}
            };
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }
        assert(caught);
    }
}

void TestValidResourceBoundaries()
{
    const auto lower = MakeUnit(UnitResources{0, 0, 7});
    assert(lower.GetHp() == 0);
    assert(lower.GetAp() == 0);
    assert(lower.IsIncapacitated());
    assert(!lower.CanAct());

    const auto upper = MakeUnit(UnitResources{78, 7, 7});
    assert(upper.GetHp() == upper.GetMaxHp());
    assert(upper.GetAp() == upper.GetMaxAp());
    assert(!upper.IsIncapacitated());
    assert(upper.CanAct());

    auto zeroMaxAp = MakeUnit(UnitResources{20, 0, 0});
    const int change = zeroMaxAp.ApplyApDelta(1);
    assert(change == 0);
    assert(zeroMaxAp.GetAp() == 0);
}

void TestHpChangesWithinBounds()
{
    auto unit = MakeUnit(UnitResources{20, 0, 0});

    unit.ApplyHpDelta(-5);
    assert(unit.GetHp() == 15);

    unit.ApplyHpDelta(3);
    assert(unit.GetHp() == 18);
}

void TestRecoveryFromIncapacitation()
{
    auto unit = MakeUnit(UnitResources{0, 0, 7});
    assert(unit.IsIncapacitated());

    unit.ApplyHpDelta(10);
    assert(!unit.IsIncapacitated());
    assert(unit.CanAct());
}

void TestInitialStateGetters()
{
    const auto unit = MakeUnit(
        UnitResources{30, 1, 3, 10}, 
        CombatUnitId{20},
        TeamId{2}
    );
    
    assert(unit.GetId().Value() == 20);
    assert(unit.GetTeam().Value() == 2);
    
    assert(unit.GetHp() == 30);
    assert(unit.GetAp() == 1);
    assert(unit.GetMaxAp() == 3);
    assert(unit.GetShield() == 10);
}

void TestCombatStatsIntegration()
{
    auto stats = MakeStats(0);
    stats.level = 10;

    const CombatUnit unit{
        CombatUnitId{10},
        TeamId{1},
        stats,
        UnitResources{}
    };

    assert(unit.GetMaxHp() == 95);
}

void TestInvalidStatsRejected()
{
    bool caught = false;
    try
    {
        auto stats = MakeStats(0);
        stats.level = 101;
        
        CombatUnit{
            CombatUnitId{10},
            TeamId{1},
            stats,
            UnitResources()
        };
    }
    catch(const std::out_of_range&)
    {
        caught = true;
    }
    assert(caught);
    
    caught = false;
    try
    {
        CombatUnit{
            CombatUnitId{10},
            TeamId{1},
            MakeStats(-1),
            UnitResources()
        };
    }
    catch(const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught);
}

int main()
{
    TestDamageAndIndependence();
    TestHealingLimit();
    TestApLimits();
    TestZeroDelta();
    TestExtremeDeltas();
    TestNegativeHpRejected();
    TestInvalidResourcesRejected();
    TestInvalidIdsRejected();
    TestValidResourceBoundaries();
    TestHpChangesWithinBounds();
    TestRecoveryFromIncapacitation();
    TestInitialStateGetters();
    TestCombatStatsIntegration();
    TestInvalidStatsRejected();
}
