#pragma once

#include <evermore/data/UnitStats.h>
#include <evermore/combat/CombatStats.h>

double GetLevelBaseHP(int level);
CombatStats CalculateCombatStats(const UnitStats& stats);