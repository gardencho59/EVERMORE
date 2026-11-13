#include <cassert>
#include <stdexcept>
#include <cmath>

#include <evermore/combat/StatCalculator.h>

void TestLevelHpTableValues()
{
    assert(GetLevelBaseHP(1) == 55.0);
    assert(GetLevelBaseHP(10) == 95.0);
    assert(GetLevelBaseHP(20) == 245.0);
    assert(GetLevelBaseHP(30) == 555.0);
    assert(GetLevelBaseHP(40) == 1025.0);
    assert(GetLevelBaseHP(50) == 1655.0);
    assert(GetLevelBaseHP(60) == 2445.0);
    assert(GetLevelBaseHP(70) == 3395.0);
    assert(GetLevelBaseHP(80) == 4505.0);
    assert(GetLevelBaseHP(90) == 5775.0);
    assert(GetLevelBaseHP(100) == 7205.0);
}

void TestLevelHpInterpolation()
{
    assert(GetLevelBaseHP(15) == 170.0);
    assert(GetLevelBaseHP(25) == 400.0);
}

void TestFractionalLevelHpInterpolation()
{
    const double actual = GetLevelBaseHP(2);
    const double expected = 55.0 + 40.0 / 9.0;

    assert(std::abs(actual - expected) < 1e-9);
}

void TestInvalidLevelsRejected()
{
    const int inValidValues[] = {-1, 0, 101};

    bool caught;
    for (const auto& level : inValidValues)
    {
        caught = false;
        try
        {
            GetLevelBaseHP(level);
        }
        catch(const std::out_of_range&)
        {
            caught = true;
        }
        assert(caught);
    }
}

void TestZeroStats()
{
    UnitStats stats(1, StatBlock{});
    CombatStats combatStats = CalculateCombatStats(stats); 

    assert(combatStats.maxHp == 55);
    assert(combatStats.physicalAttack == 5.0);
    assert(combatStats.magicAttack == 5.0);
    assert(combatStats.physicalDefense == 0.0);
    assert(combatStats.magicDefense == 0.0);
    assert(combatStats.speed == 30.0);
    assert(std::abs(combatStats.criticalChance - 0.08) < 1e-9);
    assert(combatStats.accuracyScore == 0.0);
    assert(combatStats.evasionScore == 0.0);
}

void TestMaxHpRounding()
{
    UnitStats stats{};
    stats.baseStats_.Set(StatType::Vitality, 5);

    assert(CalculateCombatStats(stats).maxHp == 69);
}

void TestMaxHpPreservesInterpolationPrecision()
{
    UnitStats stats{};
    stats.level = 2;
    stats.baseStats_.Set(StatType::Vitality, 5);

    assert(CalculateCombatStats(stats).maxHp == 74);
}

void TestStrengthContribution()
{
    UnitStats stats{};
    stats.baseStats_.Set(StatType::Strength, 25);
    const CombatStats result = CalculateCombatStats(stats);

    assert(std::abs(result.physicalAttack - 55.0) < 1e-9);
    assert(std::abs(result.physicalDefense - 62.5) < 1e-9);
}

void TestIntelligenceContribution()
{
    UnitStats stats{};
    stats.baseStats_.Set(StatType::Intelligence, 25);
    const CombatStats result = CalculateCombatStats(stats);

    assert(std::abs(result.magicAttack - 55.0) < 1e-9);
    assert(std::abs(result.magicDefense - 62.5) < 1e-9);
}

void TestVitalityDefenseContribution()
{
    UnitStats stats{};
    stats.baseStats_.Set(StatType::Vitality, 35);
    const CombatStats result = CalculateCombatStats(stats);

    assert(std::abs(result.physicalDefense - 100.0) < 1e-9);
    assert(std::abs(result.magicDefense - 100.0) < 1e-9);
}

void TestSpeedFormula()
{
    UnitStats stats{};
    stats.baseStats_.Set(StatType::Agility, 12);

    assert(std::abs(CalculateCombatStats(stats).speed - 176.0) < 1e-9);
}

void TestCriticalChanceFormula()
{
    UnitStats stats{};
    stats.baseStats_.Set(StatType::Technique, 35);

    assert(std::abs(CalculateCombatStats(stats).criticalChance - 0.39) < 1e-9);
}

void TestAccuracyAndEvasionScores()
{
    UnitStats stats{};
    stats.baseStats_.Set(StatType::Technique, 100);
    stats.baseStats_.Set(StatType::Agility, 100);
    const CombatStats result = CalculateCombatStats(stats);

    assert(std::abs(result.accuracyScore - 50.0) < 1e-9);
    assert(std::abs(result.evasionScore - 50.0) < 1e-9);
}

void TestNegativeStatsRejected()
{
    const StatType statTypes[] = {
        StatType::Strength,
        StatType::Intelligence,
        StatType::Vitality,
        StatType::Technique,
        StatType::Agility
    };

    for (const StatType type : statTypes)
    {
        UnitStats stats{};
        stats.baseStats_.Set(type, -1);
        bool caught = false;

        try
        {
            CalculateCombatStats(stats);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }

        assert(caught);
    }
}

void TestDefaultCriticalMultiplier()
{
    assert(CalculateCombatStats(UnitStats{}).criticalMultiplier == 1.5);
}

int main()
{
    TestLevelHpTableValues();
    TestLevelHpInterpolation();
    TestFractionalLevelHpInterpolation();
    TestInvalidLevelsRejected();
    TestZeroStats();
    TestMaxHpRounding();
    TestMaxHpPreservesInterpolationPrecision();
    TestStrengthContribution();
    TestIntelligenceContribution();
    TestVitalityDefenseContribution();
    TestSpeedFormula();
    TestCriticalChanceFormula();
    TestAccuracyAndEvasionScores();
    TestNegativeStatsRejected();
    TestDefaultCriticalMultiplier();
}
