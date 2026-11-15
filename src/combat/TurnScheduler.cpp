#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include <evermore/combat/TurnScheduler.h>

namespace
{
constexpr VirtualTime kActionGaugeThreshold = 1'000;
constexpr VirtualTime kTimeScale = 1'000;
constexpr VirtualTime kRoundedScale = kActionGaugeThreshold * kTimeScale;
} // namespace

void TurnScheduler::Initialize(std::span<const TurnScheduleEntry> entries)
{
    std::vector<TurnScheduleEntry> newEntries;
    newEntries.reserve(entries.size());

    for (const auto& entry : entries)
    {
        if (entry.effectiveSpeed < 1)
        {
            throw std::invalid_argument("Speed must be at least 1");
        }

        auto& added = newEntries.emplace_back(entry);

        added.actionInterval = std::max(VirtualTime{1}, kRoundedScale / added.effectiveSpeed);

        added.nextActionTime = added.actionInterval;
    }

    entries_ = std::move(newEntries);
    currentTime_ = 0;
    activeTurn_.reset();
}

std::optional<std::size_t> TurnScheduler::FindNextEntryIndex() const
{
    auto it = std::min_element(entries_.begin(),
                               entries_.end(),
                               [](const TurnScheduleEntry& a, const TurnScheduleEntry& b)
                               {
                                   if (a.active != b.active)
                                   {
                                       return a.active;
                                   }
                                   if (a.nextActionTime != b.nextActionTime)
                                   {
                                       return a.nextActionTime < b.nextActionTime;
                                   }
                                   if (a.effectiveSpeed != b.effectiveSpeed)
                                   {
                                       return a.effectiveSpeed > b.effectiveSpeed;
                                   }

                                   return a.unitId < b.unitId;
                               });

    if (it == entries_.end() || !it->active)
    {
        return std::nullopt;
    }

    return static_cast<std::size_t>(it - entries_.begin());
}

std::optional<CombatUnitId> TurnScheduler::PeekNextUnit() const
{
    if (activeTurn_)
    {
        return activeTurn_;
    }

    auto index = FindNextEntryIndex();
    if (!index)
    {
        return std::nullopt;
    }

    return entries_[*index].unitId;
}

std::optional<CombatUnitId> TurnScheduler::BeginNextTurn()
{
    if (activeTurn_)
    {
        return std::nullopt;
    }

    auto index = FindNextEntryIndex();
    if (!index)
    {
        return std::nullopt;
    }

    const auto& entry = entries_[*index];

    currentTime_ = entry.nextActionTime;
    activeTurn_ = entry.unitId;

    return entry.unitId;
}

bool TurnScheduler::CompleteTurn(CombatUnitId unitId)
{
    if (!activeTurn_ || *activeTurn_ != unitId)
    {
        return false;
    }

    auto it = std::ranges::find(entries_, unitId, &TurnScheduleEntry::unitId);

    if (it == entries_.end())
    {
        return false;
    }

    const auto maxTime = std::numeric_limits<VirtualTime>::max();

    if (currentTime_ > maxTime - it->actionInterval)
    {
        throw std::overflow_error("Virtual time overflow");
    }

    it->nextActionTime = it->actionInterval + currentTime_;

    activeTurn_.reset();
    return true;
}

bool TurnScheduler::UpdateSpeed(CombatUnitId unitId, int effectiveSpeed)
{
    if (effectiveSpeed < 1)
    {
        return false;
    }

    auto it = std::ranges::find(entries_, unitId, &TurnScheduleEntry::unitId);

    if (it == entries_.end())
    {
        return false;
    }

    const auto newInterval = std::max(VirtualTime{1}, kRoundedScale / effectiveSpeed);

    const bool isTakingTurn = activeTurn_ && *activeTurn_ == unitId;

    if (isTakingTurn || !it->active)
    {
        it->effectiveSpeed = effectiveSpeed;
        it->actionInterval = newInterval;
        return true;
    }

    const auto oldInterval = it->actionInterval;
    const auto remainingTime = it->nextActionTime - currentTime_;
    const auto maxTime = std::numeric_limits<VirtualTime>::max();

    if (remainingTime > maxTime / newInterval)
    {
        throw std::overflow_error("Virtual time multiplication overflow");
    }

    const auto newRemainingTime = remainingTime * newInterval / oldInterval;

    if (currentTime_ > maxTime - newRemainingTime)
    {
        throw std::overflow_error("Virtual time addition overflow");
    }

    it->nextActionTime = currentTime_ + newRemainingTime;
    it->actionInterval = newInterval;
    it->effectiveSpeed = effectiveSpeed;

    return true;
}

bool TurnScheduler::SetActive(CombatUnitId unitId, bool active)
{
    auto it = std::ranges::find(entries_, unitId, &TurnScheduleEntry::unitId);

    if (it == entries_.end())
    {
        return false;
    }

    if (!it->active && active)
    {
        const auto maxTime = std::numeric_limits<VirtualTime>::max();

        if (currentTime_ > maxTime - it->actionInterval)
        {
            throw std::overflow_error("Virtual time addition overflow");
        }
        it->nextActionTime = currentTime_ + it->actionInterval;
    }

    it->active = active;
    return true;
}

std::vector<CombatUnitId> TurnScheduler::PreviewTurns(std::size_t count) const
{
    TurnScheduler preview = *this;
    std::vector<CombatUnitId> previewUnits;
    previewUnits.reserve(count);

    for (std::size_t i = 0; i < count; ++i)
    {
        const auto nextUnit = preview.activeTurn_ ? preview.activeTurn_ : preview.BeginNextTurn();

        if (!nextUnit)
        {
            break;
        }
        preview.CompleteTurn(*nextUnit);
        previewUnits.push_back(*nextUnit);
    }

    return previewUnits;
}

VirtualTime TurnScheduler::GetCurrentTime() const noexcept
{
    return currentTime_;
}
