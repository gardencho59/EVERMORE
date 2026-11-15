#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

#include <evermore/combat/TurnScheduler.h>

std::vector<TurnScheduleEntry> MakeEntries(const std::vector<int>& speeds = {100})
{
    std::vector<TurnScheduleEntry> entries;
    entries.reserve(speeds.size());

    for (std::size_t i = 0; i < speeds.size(); ++i)
    {
        TurnScheduleEntry entry{CombatUnitId{static_cast<CombatUnitId::value_type>(i + 1)}};
        entry.effectiveSpeed = speeds[i];
        entries.push_back(entry);
    }

    return entries;
}

void TestInitializeResetsState()
{
    const auto entries = MakeEntries();

    TurnScheduler scheduler;
    scheduler.Initialize(entries);

    assert(scheduler.GetCurrentTime() == 0);

    const auto unit = scheduler.BeginNextTurn();
    assert(unit.has_value());
    assert(scheduler.GetCurrentTime() > 0);

    scheduler.Initialize(entries);

    assert(scheduler.GetCurrentTime() == 0);

    const auto nextUnit = scheduler.BeginNextTurn();
    assert(nextUnit.has_value());
    assert(nextUnit == unit);
    assert(scheduler.GetCurrentTime() == 10'000);
}

void TestInitializeRejectsInvalidSpeed()
{
    for (const int speed : {0, -1})
    {
        bool caught = false;

        const auto entries = MakeEntries({speed});

        TurnScheduler scheduler;
        try
        {
            scheduler.Initialize(entries);
        }
        catch (const std::invalid_argument&)
        {
            caught = true;
        }

        assert(caught);
    }
}

void TestEmptyScheduler()
{
    const auto entries = MakeEntries({});

    TurnScheduler scheduler;
    scheduler.Initialize(entries);

    const auto unit = scheduler.BeginNextTurn();
    assert(!unit);
    assert(scheduler.PreviewTurns(5).empty());
}

void TestAllUnitsInactive()
{
    const auto entries = MakeEntries({100, 200, 300});

    TurnScheduler scheduler;
    scheduler.Initialize(entries);

    for (const TurnScheduleEntry& entry : entries)
    {
        const bool changed = scheduler.SetActive(entry.unitId, false);
        assert(changed);
    }

    const auto unit = scheduler.BeginNextTurn();

    assert(!unit);

    assert(scheduler.PreviewTurns(5).empty());
}

void TestFasterUnitActsFirst()
{
    const auto entries = MakeEntries({100, 200});

    TurnScheduler scheduler;
    scheduler.Initialize(entries);

    const auto unit = scheduler.PeekNextUnit();
    assert(unit.has_value());
    assert(unit->Value() == 2);
}

void TestEqualTimePrefersHigherSpeed()
{
    const auto entries = MakeEntries({100, 200});

    TurnScheduler scheduler;
    scheduler.Initialize(entries);

    const auto unitHigherSpeed = scheduler.BeginNextTurn();
    assert(unitHigherSpeed.has_value());
    assert(unitHigherSpeed->Value() == 2);
    const bool completed = scheduler.CompleteTurn(*unitHigherSpeed);
    assert(completed);

    const auto nextUnit = scheduler.BeginNextTurn();
    assert(nextUnit.has_value());
    assert(nextUnit->Value() == 2);
}

void TestEqualTimeAndSpeedPrefersLowerId()
{
    auto entries = MakeEntries({100, 100});
    std::swap(entries[0], entries[1]);

    TurnScheduler scheduler;
    scheduler.Initialize(entries);

    const auto unitLowerId = scheduler.BeginNextTurn();
    assert(unitLowerId.has_value());
    assert(unitLowerId->Value() == 1);
}

void TestInitializeReplacesEntries()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    scheduler.Initialize(MakeEntries({50}));

    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{1});
    assert(scheduler.GetCurrentTime() == 20'000);
    const bool updated = scheduler.UpdateSpeed(CombatUnitId{2}, 100);
    assert(!updated);
}

void TestFailedInitializePreservesState()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    const auto unit = scheduler.BeginNextTurn();
    assert(unit.has_value());
    const auto expected = scheduler.PreviewTurns(6);

    bool caught = false;
    try
    {
        scheduler.Initialize(MakeEntries({300, 0}));
    }
    catch (const std::invalid_argument&)
    {
        caught = true;
    }
    assert(caught);
    assert(scheduler.GetCurrentTime() == 5'000);
    assert(scheduler.PeekNextUnit() == unit);
    assert(scheduler.PreviewTurns(6) == expected);
    const bool completed = scheduler.CompleteTurn(*unit);
    assert(completed);
}

void TestBeginTurnAdvancesCurrentTime()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100}));
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{1});
    assert(scheduler.GetCurrentTime() == 10'000);
}

void TestCannotBeginWhileTurnIsActive()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries());
    const auto unit = scheduler.BeginNextTurn();
    assert(unit.has_value());
    const auto second = scheduler.BeginNextTurn();
    assert(!second);
    assert(scheduler.GetCurrentTime() == 10'000);
    const bool completed = scheduler.CompleteTurn(*unit);
    assert(completed);
}

void TestCompleteTurnSchedulesNextAction()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries());
    const auto unit = scheduler.BeginNextTurn();
    assert(unit.has_value());
    const bool completed = scheduler.CompleteTurn(*unit);
    assert(completed);
    assert(scheduler.GetCurrentTime() == 10'000);
    const auto next = scheduler.BeginNextTurn();
    assert(next == unit);
    assert(scheduler.GetCurrentTime() == 20'000);
}

void TestCompleteTurnRejectsWrongUnit()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{2});
    const bool wrongUnit = scheduler.CompleteTurn(CombatUnitId{1});
    const bool unknownUnit = scheduler.CompleteTurn(CombatUnitId{99});
    assert(!wrongUnit);
    assert(!unknownUnit);
    assert(scheduler.PeekNextUnit() == unit);
    assert(scheduler.GetCurrentTime() == 5'000);
    const bool completed = scheduler.CompleteTurn(*unit);
    assert(completed);
}

void TestCompleteTurnWithoutActiveTurn()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries());
    const bool completed = scheduler.CompleteTurn(CombatUnitId{1});
    assert(!completed);
    assert(scheduler.GetCurrentTime() == 0);
}

void TestUpdateSpeedRescalesRemainingTime()
{
    for (const int speed : {50, 200})
    {
        TurnScheduler scheduler;
        scheduler.Initialize(MakeEntries({100, 200}));
        const auto first = scheduler.BeginNextTurn();
        assert(first == CombatUnitId{2});
        const bool completed = scheduler.CompleteTurn(*first);
        const bool disabled = scheduler.SetActive(*first, false);
        assert(completed);
        assert(disabled);

        const bool updated = scheduler.UpdateSpeed(CombatUnitId{1}, speed);
        assert(updated);
        assert(scheduler.GetCurrentTime() == 5'000);
        const auto next = scheduler.BeginNextTurn();
        assert(next == CombatUnitId{1});
        const VirtualTime expectedTime = speed == 50 ? 15'000 : 7'500;
        assert(scheduler.GetCurrentTime() == expectedTime);
    }
}

void TestUpdateSpeedDuringActiveTurn()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries());
    const auto unit = scheduler.BeginNextTurn();
    assert(unit.has_value());
    const bool updated = scheduler.UpdateSpeed(*unit, 200);
    assert(updated);
    assert(scheduler.GetCurrentTime() == 10'000);
    assert(scheduler.PeekNextUnit() == unit);
    const bool completed = scheduler.CompleteTurn(*unit);
    assert(completed);
    const auto next = scheduler.BeginNextTurn();
    assert(next == unit);
    assert(scheduler.GetCurrentTime() == 15'000);
}

void TestUpdateSpeedWhileInactive()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    const bool disabled = scheduler.SetActive(CombatUnitId{1}, false);
    const bool updated = scheduler.UpdateSpeed(CombatUnitId{1}, 50);
    assert(disabled);
    assert(updated);
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{2});
    const bool completed = scheduler.CompleteTurn(*unit);
    const bool otherDisabled = scheduler.SetActive(*unit, false);
    const bool enabled = scheduler.SetActive(CombatUnitId{1}, true);
    assert(completed);
    assert(otherDisabled);
    assert(enabled);
    const auto next = scheduler.BeginNextTurn();
    assert(next == CombatUnitId{1});
    assert(scheduler.GetCurrentTime() == 25'000);
}

void TestUpdateSpeedRejectsInvalidInput()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    const auto expected = scheduler.PreviewTurns(6);
    for (const int speed : {0, -1})
    {
        const bool updated = scheduler.UpdateSpeed(CombatUnitId{1}, speed);
        assert(!updated);
    }
    const bool unknown = scheduler.UpdateSpeed(CombatUnitId{99}, 300);
    assert(!unknown);
    assert(scheduler.GetCurrentTime() == 0);
    assert(scheduler.PreviewTurns(6) == expected);
}

void TestInactiveUnitIsSkipped()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    const bool disabled = scheduler.SetActive(CombatUnitId{2}, false);
    assert(disabled);
    const std::vector<CombatUnitId> expected(5, CombatUnitId{1});
    assert(scheduler.PreviewTurns(5) == expected);
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{1});
}

void TestReactivationSchedulesFromCurrentTime()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 100}));
    const bool disabled = scheduler.SetActive(CombatUnitId{1}, false);
    assert(disabled);
    for (int i = 0; i < 3; ++i)
    {
        const auto unit = scheduler.BeginNextTurn();
        assert(unit == CombatUnitId{2});
        const bool completed = scheduler.CompleteTurn(*unit);
        assert(completed);
    }
    assert(scheduler.GetCurrentTime() == 30'000);
    const bool enabled = scheduler.SetActive(CombatUnitId{1}, true);
    assert(enabled);
    const auto next = scheduler.BeginNextTurn();
    assert(next == CombatUnitId{1});
    assert(scheduler.GetCurrentTime() == 40'000);
}

void TestSetActiveWithSameStatePreservesSchedule()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{2});
    const bool completed = scheduler.CompleteTurn(*unit);
    assert(completed);
    const auto expected = scheduler.PreviewTurns(6);
    const bool enabled = scheduler.SetActive(CombatUnitId{1}, true);
    assert(enabled);
    assert(scheduler.PreviewTurns(6) == expected);

    const bool disabled = scheduler.SetActive(CombatUnitId{1}, false);
    assert(disabled);
    const auto inactiveExpected = scheduler.PreviewTurns(6);
    const bool disabledAgain = scheduler.SetActive(CombatUnitId{1}, false);
    assert(disabledAgain);
    assert(scheduler.PreviewTurns(6) == inactiveExpected);
    assert(scheduler.GetCurrentTime() == 5'000);
}

void TestSetActiveRejectsUnknownUnit()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries());
    const auto expected = scheduler.PreviewTurns(5);
    const bool disabled = scheduler.SetActive(CombatUnitId{99}, false);
    const bool enabled = scheduler.SetActive(CombatUnitId{99}, true);
    assert(!disabled);
    assert(!enabled);
    assert(scheduler.PreviewTurns(5) == expected);
}

void TestDeactivateCurrentUnitThenCompleteTurn()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({200, 100}));
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{1});
    const bool disabled = scheduler.SetActive(*unit, false);
    assert(disabled);
    assert(scheduler.PeekNextUnit() == unit);
    const auto blocked = scheduler.BeginNextTurn();
    assert(!blocked);
    const bool completed = scheduler.CompleteTurn(*unit);
    assert(completed);
    const std::vector<CombatUnitId> expected(5, CombatUnitId{2});
    assert(scheduler.PreviewTurns(5) == expected);
    const auto next = scheduler.BeginNextTurn();
    assert(next == CombatUnitId{2});
}

void TestPreviewReturnsRequestedCount()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({200, 100}));
    assert(scheduler.PreviewTurns(1).size() == 1);
    const std::vector<CombatUnitId> expected{CombatUnitId{1},
                                             CombatUnitId{1},
                                             CombatUnitId{2},
                                             CombatUnitId{1},
                                             CombatUnitId{1},
                                             CombatUnitId{2}};
    assert(scheduler.PreviewTurns(6) == expected);
}

void TestPreviewRepeatsSingleUnit()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries());
    const std::vector<CombatUnitId> expected(5, CombatUnitId{1});
    assert(scheduler.PreviewTurns(5) == expected);
}

void TestPreviewWithZeroCount()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries());
    const auto unit = scheduler.BeginNextTurn();
    assert(unit.has_value());
    assert(scheduler.PreviewTurns(0).empty());
    assert(scheduler.PeekNextUnit() == unit);
}

void TestPreviewIncludesCurrentTurn()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({200, 100}));
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{1});
    const std::vector<CombatUnitId> expected{CombatUnitId{1}, CombatUnitId{1}, CombatUnitId{2}};
    assert(scheduler.PreviewTurns(3) == expected);
}

void TestPreviewMatchesActualTurnOrder()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({75, 200, 130}));
    const auto expected = scheduler.PreviewTurns(20);
    std::vector<CombatUnitId> actual;
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        const auto unit = scheduler.BeginNextTurn();
        assert(unit.has_value());
        actual.push_back(*unit);
        const bool completed = scheduler.CompleteTurn(*unit);
        assert(completed);
    }
    assert(actual == expected);
}

void TestPreviewPreservesSchedulerState()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    const auto unit = scheduler.BeginNextTurn();
    assert(unit.has_value());
    const auto first = scheduler.PreviewTurns(10);
    assert(scheduler.PreviewTurns(10) == first);
    assert(scheduler.GetCurrentTime() == 5'000);
    const auto blocked = scheduler.BeginNextTurn();
    assert(!blocked);
    const bool completed = scheduler.CompleteTurn(*unit);
    assert(completed);
    for (std::size_t i = 1; i < first.size(); ++i)
    {
        const auto next = scheduler.BeginNextTurn();
        assert(next == first[i]);
        const bool nextCompleted = scheduler.CompleteTurn(*next);
        assert(nextCompleted);
    }
}

void TestPeekReturnsNextUnitWithoutChangingState()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 200}));
    assert(scheduler.PeekNextUnit() == CombatUnitId{2});
    assert(scheduler.PeekNextUnit() == CombatUnitId{2});
    assert(scheduler.GetCurrentTime() == 0);
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{2});
    assert(scheduler.GetCurrentTime() == 5'000);
}

void TestPeekReturnsCurrentUnitDuringActiveTurn()
{
    TurnScheduler scheduler;
    scheduler.Initialize(MakeEntries({100, 100}));
    const auto unit = scheduler.BeginNextTurn();
    assert(unit == CombatUnitId{1});
    const bool updated = scheduler.UpdateSpeed(*unit, 50);
    assert(updated);
    assert(scheduler.PeekNextUnit() == unit);
    assert(scheduler.GetCurrentTime() == 10'000);
}

void TestPeekWithNoActiveUnits()
{
    TurnScheduler scheduler;
    assert(!scheduler.PeekNextUnit());
    scheduler.Initialize(MakeEntries());
    const bool disabled = scheduler.SetActive(CombatUnitId{1}, false);
    assert(disabled);
    assert(!scheduler.PeekNextUnit());
}

int main()
{
    TestInitializeResetsState();
    TestInitializeRejectsInvalidSpeed();
    TestEmptyScheduler();
    TestAllUnitsInactive();
    TestFasterUnitActsFirst();
    TestEqualTimePrefersHigherSpeed();
    TestEqualTimeAndSpeedPrefersLowerId();
    TestInitializeReplacesEntries();
    TestFailedInitializePreservesState();
    TestBeginTurnAdvancesCurrentTime();
    TestCannotBeginWhileTurnIsActive();
    TestCompleteTurnSchedulesNextAction();
    TestCompleteTurnRejectsWrongUnit();
    TestCompleteTurnWithoutActiveTurn();
    TestUpdateSpeedRescalesRemainingTime();
    TestUpdateSpeedDuringActiveTurn();
    TestUpdateSpeedWhileInactive();
    TestUpdateSpeedRejectsInvalidInput();
    TestInactiveUnitIsSkipped();
    TestReactivationSchedulesFromCurrentTime();
    TestSetActiveWithSameStatePreservesSchedule();
    TestSetActiveRejectsUnknownUnit();
    TestDeactivateCurrentUnitThenCompleteTurn();
    TestPreviewReturnsRequestedCount();
    TestPreviewRepeatsSingleUnit();
    TestPreviewWithZeroCount();
    TestPreviewIncludesCurrentTurn();
    TestPreviewMatchesActualTurnOrder();
    TestPreviewPreservesSchedulerState();
    TestPeekReturnsNextUnitWithoutChangingState();
    TestPeekReturnsCurrentUnitDuringActiveTurn();
    TestPeekWithNoActiveUnits();
}
