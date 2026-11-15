#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include <evermore/core/StrongId.h>

using VirtualTime = std::int64_t;

struct TurnScheduleEntry
{
    CombatUnitId unitId;

    VirtualTime nextActionTime = 0;
    VirtualTime actionInterval = 1;

    int effectiveSpeed = 1;
    bool active = true;
};

class TurnScheduler
{
public:
    void Initialize(std::span<const TurnScheduleEntry> entries);

    std::optional<CombatUnitId> PeekNextUnit() const;

    std::optional<CombatUnitId> BeginNextTurn();

    bool CompleteTurn(CombatUnitId unitId);

    bool UpdateSpeed(CombatUnitId unitId, int effectiveSpeed);

    bool SetActive(CombatUnitId unitId, bool active);

    std::vector<CombatUnitId> PreviewTurns(std::size_t count) const;

    VirtualTime GetCurrentTime() const noexcept;

private:
    std::optional<std::size_t> FindNextEntryIndex() const;

    VirtualTime currentTime_ = 0;

    std::optional<CombatUnitId> activeTurn_;

    std::vector<TurnScheduleEntry> entries_;
};