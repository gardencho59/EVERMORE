#include <cassert>
#include <stdexcept>
#include <type_traits>

#include <evermore/combat/CombatState.h>

CombatUnit MakeUnit(CombatUnitId::value_type id = 10,
                    TeamId::value_type team = 1)
{
    return CombatUnit{
        CombatUnitId{id},
        TeamId{team},
        UnitStats{},
        UnitResources{}
    };
}

void TestEmptyState()
{
    CombatState state{std::vector<CombatUnit>{}};

    assert(state.GetAllUnits().empty());
    assert(state.FindUnit(CombatUnitId{10}) == nullptr);
    assert(state.GetUnitsByTeam(TeamId{1}).empty());
}

void TestInitialPhase()
{
    CombatState state{std::vector<CombatUnit>{}};
    assert(state.GetPhase() == CombatPhase::Initializing);
}

void TestSingleUnitConstruction()
{
    CombatState state{std::vector<CombatUnit>{MakeUnit()}};
    
    assert(state.GetAllUnits().size() == 1);
}

void TestAllUnitsPreserveInputOrder()
{
    CombatState state{
        std::vector<CombatUnit>{
            MakeUnit(30),
            MakeUnit(10),
            MakeUnit(70)
        }
    };

    const auto units = state.GetAllUnits();

    assert(units.size() == 3);
    assert(units[0].GetId().Value() == 30);
    assert(units[1].GetId().Value() == 10);
    assert(units[2].GetId().Value() == 70);
}

void TestDuplicateUnitIdsRejected()
{
    bool caught = false;

    try
    {
        CombatState{
            std::vector<CombatUnit>{
                MakeUnit(10),
                MakeUnit(10, 2)
            }
        };   
    }
    catch(const std::invalid_argument&)
    {
        caught = true;
    }

    assert(caught);
}

void TestFindUnitById()
{
    CombatState state{std::vector<CombatUnit>{
        MakeUnit(30), MakeUnit(10), MakeUnit(70)
    }};

    for (const CombatUnitId id : {CombatUnitId{30}, CombatUnitId{10}, CombatUnitId{70}})
    {
        CombatUnit* unit = state.FindUnit(id);
        assert(unit != nullptr);
        assert(unit->GetId() == id);
    }
}

void TestFindUnitOnConstState()
{
    const CombatState state{std::vector<CombatUnit>{MakeUnit(30, 2)}};
    static_assert(std::is_same_v<
        decltype(state.FindUnit(CombatUnitId{30})), const CombatUnit*>);

    const CombatUnit* unit = state.FindUnit(CombatUnitId{30});
    assert(unit != nullptr);
    assert(unit->GetId() == CombatUnitId{30});
    assert(unit->GetTeam() == TeamId{2});
    assert(unit->GetHp() == 1);
}

void TestMissingUnitReturnsNull()
{
    CombatState state{std::vector<CombatUnit>{MakeUnit()}};
    assert(state.FindUnit(CombatUnitId{99}) == nullptr);

    const CombatState& constState = state;
    assert(constState.FindUnit(CombatUnitId{99}) == nullptr);
}

void TestInvalidUnitIdReturnsNull()
{
    CombatState state{std::vector<CombatUnit>{MakeUnit()}};
    assert(state.FindUnit(CombatUnitId{0}) == nullptr);

    const CombatState& constState = state;
    assert(constState.FindUnit(CombatUnitId{0}) == nullptr);
}

void TestFoundUnitChangesStoredState()
{
    CombatState state{std::vector<CombatUnit>{MakeUnit(10), MakeUnit(30)}};
    CombatUnit* unit = state.FindUnit(CombatUnitId{10});
    assert(unit != nullptr);
    assert(unit->GetHp() == 1);

    const int change = unit->ApplyHpDelta(9);
    assert(change == 9);

    const CombatUnit* updatedUnit = state.FindUnit(CombatUnitId{10});
    const CombatUnit* otherUnit = state.FindUnit(CombatUnitId{30});
    assert(updatedUnit != nullptr);
    assert(otherUnit != nullptr);
    assert(updatedUnit->GetHp() == 10);
    assert(state.GetAllUnits()[0].GetHp() == 10);
    assert(otherUnit->GetHp() == 1);
}

void TestIncapacitatedUnitRemainsFindable()
{
    CombatState state{std::vector<CombatUnit>{MakeUnit()}};
    CombatUnit* unit = state.FindUnit(CombatUnitId{10});
    assert(unit != nullptr);
    const int change = unit->ApplyHpDelta(-1);
    assert(change == -1);

    const CombatUnit* incapacitatedUnit = state.FindUnit(CombatUnitId{10});
    assert(incapacitatedUnit != nullptr);
    assert(incapacitatedUnit->GetHp() == 0);
    assert(incapacitatedUnit->IsIncapacitated());
    assert(state.GetAllUnits().size() == 1);
}

void TestGetUnitsByTeam()
{
    const CombatState state{std::vector<CombatUnit>{
        MakeUnit(10, 1), MakeUnit(30, 2), MakeUnit(70, 1)
    }};

    const auto allies = state.GetUnitsByTeam(TeamId{1});
    const auto enemies = state.GetUnitsByTeam(TeamId{2});
    assert(allies.size() == 2);
    assert(allies[0] == CombatUnitId{10});
    assert(allies[1] == CombatUnitId{70});
    assert(enemies.size() == 1);
    assert(enemies[0] == CombatUnitId{30});
}

void TestUnknownTeamReturnsEmpty()
{
    const CombatState state{std::vector<CombatUnit>{MakeUnit(10, 1)}};
    assert(state.GetUnitsByTeam(TeamId{99}).empty());
}

void TestTeamQueryIncludesIncapacitatedUnits()
{
    CombatState state{std::vector<CombatUnit>{
        MakeUnit(10, 1), MakeUnit(30, 2), MakeUnit(70, 1)
    }};
    CombatUnit* unit = state.FindUnit(CombatUnitId{70});
    assert(unit != nullptr);
    const int change = unit->ApplyHpDelta(-1);
    assert(change == -1);
    assert(unit->IsIncapacitated());

    const auto allies = state.GetUnitsByTeam(TeamId{1});
    assert(allies.size() == 2);
    assert(allies[0] == CombatUnitId{10});
    assert(allies[1] == CombatUnitId{70});
}


int main()
{
    TestEmptyState();
    TestInitialPhase();

    TestSingleUnitConstruction();
    TestAllUnitsPreserveInputOrder();
    TestDuplicateUnitIdsRejected();

    TestFindUnitById();
    TestFindUnitOnConstState();
    TestMissingUnitReturnsNull();
    TestInvalidUnitIdReturnsNull();

    TestFoundUnitChangesStoredState();
    TestIncapacitatedUnitRemainsFindable();

    TestGetUnitsByTeam();
    TestUnknownTeamReturnsEmpty();
    TestTeamQueryIncludesIncapacitatedUnits();
}
