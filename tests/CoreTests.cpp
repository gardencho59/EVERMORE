#include <cassert>
#include <unordered_map>
#include <type_traits>

#include <evermore/core/StrongId.h>

struct OtherTag {};
using OtherId = StrongId<OtherTag>;

static_assert(
    !std::is_convertible_v<OtherId, UnitDefinitionId>
);

void TestIdValue()
{
    const UnitDefinitionId id{10};
    
    assert(id.Value() == 10);
}

void TestIdEquality()
{
    const UnitDefinitionId first{10};
    const UnitDefinitionId same{10};
    const UnitDefinitionId other{20};

    assert(first == same);
    assert(first != other);
}

void TestIdValidation()
{
    assert(!UnitDefinitionId{0}.IsValid());
    assert(UnitDefinitionId{10}.IsValid());
}

void TestHashKey()
{
    std::unordered_map<UnitDefinitionId, int> values;
    values.emplace(UnitDefinitionId{10}, 42);

    assert(values.at(UnitDefinitionId{10}) == 42);
}

int main()
{
    TestIdValue();
    TestIdEquality();
    TestIdValidation();
    TestHashKey();
}