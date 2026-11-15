// WrongGPU edition: tests of OptiScaler/misc/WrongGpuHostEvent.h (what the version box does with WrongGPU's result, and
// the list order of WrongGPU's model entries against the copied wg_model_entries.h). Built and run by
// .github/workflows/wronggpu-edition-tests.yml:  cl /EHsc /std:c++17 /I OptiScaler/misc host_event_test.cpp

#include "WrongGpuHostEvent.h"

#include <cstdio>

static int g_checks = 0, g_failures = 0;
#define CHECK(c)                                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        ++g_checks;                                                                                                    \
        if (!(c))                                                                                                      \
        {                                                                                                              \
            ++g_failures;                                                                                              \
            std::printf("FAIL line %d: %s\n", __LINE__, #c);                                                           \
        }                                                                                                              \
    } while (0)

using namespace wronggpu_edition;

static int32_t g_lastVersion = 0;
static int32_t FakeHostSelected(uint64_t versionId)
{
    g_lastVersion = static_cast<int32_t>(versionId);
    return versionId == 7 ? 1 : versionId == 9 ? -1 : 0;
}

int main()
{
    // 1: this model runs from now on: recreate, also when the entry was already selected
    auto a = OnVersionSelected(2, 2, 1);
    CHECK(a.setIndex && a.recreate && !a.notSaved);
    a = OnVersionSelected(0, 2, 1);
    CHECK(a.setIndex && a.recreate && !a.notSaved);
    // 0: as before: a new entry recreates, the entry already selected does nothing
    a = OnVersionSelected(0, 2, 0);
    CHECK(a.setIndex && a.recreate && !a.notSaved);
    a = OnVersionSelected(2, 2, 0);
    CHECK(!a.setIndex && !a.recreate && !a.notSaved);
    // negative: not saved: nothing changes (no new index, no recreate), the player is told
    for (int32_t r : { -1, -2, -3 })
    {
        a = OnVersionSelected(0, 2, r);
        CHECK(!a.setIndex && !a.recreate && a.notSaved);
        a = OnVersionSelected(2, 2, r);
        CHECK(!a.setIndex && !a.recreate && a.notSaved);
    }

    // the export: called when found, 0 without it
    CHECK(CallHostSelected([]() -> PfnHostSelected { return nullptr; }, 7) == 0);
    CHECK(CallHostSelected([]() -> PfnHostSelected { return &FakeHostSelected; }, 7) == 1 && g_lastVersion == 7);
    CHECK(CallHostSelected([]() -> PfnHostSelected { return &FakeHostSelected; }, 9) == -1 && g_lastVersion == 9);

    // the names WrongGPU's dll lists
    CHECK(ModelOfName("WrongGPU Model K##3.1.0") == 'K' && ModelOfName("WrongGPU Model M") == 'M');
    CHECK(ModelOfName("WrongGPU Model Q") == 0 && ModelOfName("FSR 3.1.4") == 0 && ModelOfName(nullptr) == 0);
    CHECK(ModelOfName("WrongGPU Model LX") == 0);

    // the list order against wg_model_entries.h: M, then K, then L, as each build lists them (AMD's versions follow)
    CHECK(ListOrderMatches({ "WrongGPU Model M##3.1.0", "WrongGPU Model K##3.1.0", "WrongGPU Model L##3.1.0", "3.1.5" }));
    CHECK(ListOrderMatches({ "WrongGPU Model M##3.1.0", "WrongGPU Model L##3.1.0", "3.1.5", "2.3.4" }));
    CHECK(ListOrderMatches({ "WrongGPU Model M##3.1.0", "WrongGPU Model K##3.1.0" }));
    CHECK(ListOrderMatches({ "FSR 3.1.4", "FSR 2.3.4" }));  // not WrongGPU's dll
    CHECK(!ListOrderMatches({ "WrongGPU Model K##3.1.0", "WrongGPU Model M##3.1.0" }));
    CHECK(!ListOrderMatches({ "WrongGPU Model M##3.1.0", "WrongGPU Model L##3.1.0", "WrongGPU Model K##3.1.0" }));
    CHECK(!ListOrderMatches({ "3.1.5", "WrongGPU Model M##3.1.0" }));

    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures ? 1 : 0;
}
