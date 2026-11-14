#pragma once

#include <evermore/combat/CombatStats.h>
#include <evermore/data/UnitStats.h>

double GetLevelBaseHP(int level);
CombatStats CalculateCombatStats(const UnitStats& stats);