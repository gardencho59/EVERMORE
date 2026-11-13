#pragma once

struct CombatStats
{
    int maxHp = 0;
    
    double physicalAttack = 0.0;
    double magicAttack = 0.0;
    double physicalDefense = 0.0;
    double magicDefense = 0.0;
    double speed = 0.0;

    double criticalChance = 0.0;
    double criticalMultiplier = 1.5;

    double accuracyScore = 0.0;
    double evasionScore = 0.0;
};