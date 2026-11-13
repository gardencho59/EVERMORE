#include <array>
#include <cmath>
#include <stdexcept>

#include <evermore/combat/StatCalculator.h>

struct LevelHpEntry
{
    int level;
    int baseHp;
};

constexpr std::array<LevelHpEntry, 11> kLevelHpTable{{
    {1, 55},
    {10, 95},
    {20, 245},
    {30, 555},
    {40, 1025},
    {50, 1655},
    {60, 2445},
    {70, 3395},
    {80, 4505},
    {90, 5775},
    {100, 7205},
}};

double GetLevelBaseHP(int level)
{
    if (level < kLevelHpTable.front().level ||
        level > kLevelHpTable.back().level)
    {
        throw std::out_of_range("Unsupported level");        
    }

    for (std::size_t i = 1; i < kLevelHpTable.size(); ++i)
    {
        const auto& lower = kLevelHpTable[i - 1];
        const auto& upper = kLevelHpTable[i];

        if (level <= upper.level)
        {
            const double ratio = static_cast<double>(level - lower.level) / (upper.level - lower.level);

            return lower.baseHp + (upper.baseHp - lower.baseHp) * ratio;
        }
    }

    return kLevelHpTable.back().baseHp;
};

CombatStats CalculateCombatStats(const UnitStats& stats)
{
    CombatStats combatStats;

    const double strength = stats.baseStats_.Get(StatType::Strength);
    const double intelligence = stats.baseStats_.Get(StatType::Intelligence);
    const double vitality = stats.baseStats_.Get(StatType::Vitality);
    const double technique = stats.baseStats_.Get(StatType::Technique);
    const double agility = stats.baseStats_.Get(StatType::Agility);

    if (strength < 0 || intelligence < 0 || vitality < 0 || technique < 0 || agility < 0)
    {
        throw std::invalid_argument("Stats cannot be negative");
    }

    const double levelBaseHp = GetLevelBaseHP(stats.level);
    combatStats.maxHp = static_cast<int>(std::round(
        levelBaseHp * (1 + 2 * vitality / (35 + vitality))
    ));

    combatStats.physicalAttack = 5 + (100 * strength / (25 + strength));
    combatStats.magicAttack = 5 + (100 * intelligence / (25 + intelligence));

    combatStats.physicalDefense = 150 * (strength + 2 * vitality) / (35 + strength + 2 * vitality);
    combatStats.magicDefense = 150 * (intelligence + 2 * vitality) / (35 + intelligence + 2 * vitality);

    combatStats.speed = 30 + (220 * agility * agility) / (144 + agility * agility) + 3 * agility;

    combatStats.criticalChance = (8 + 62 * technique / (35 + technique)) / 100.0;

    combatStats.accuracyScore = 100 * technique / (100 + technique);
    combatStats.evasionScore = 100 * agility / (100 + agility);

    return combatStats;
};