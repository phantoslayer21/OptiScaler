#pragma once

// WrongGPU edition: what the version box does with the player's selection when the upscaler's dll is WrongGPU's FFX dll
// (WRONGGPU-EDITION.md item 6). Kept free of ImGui and Windows so wronggpu-edition-tests/host_event_test.cpp can test
// it.
//
// include/wronggpu/wg_model_entries.h is a byte-identical copy of WrongGPU's clean/wgshared/include/wg_model_entries.h
// (the order of WrongGPU's model entries in the list its dll returns); WrongGPU's CI checks that the copies match.

#include "../include/wronggpu/wg_model_entries.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace wronggpu_edition
{

// The order the copied header defines, as this edition expects it.
static_assert(wgentries::ModelEntryIndex('M', true, true) == 0, "Model M is the first WrongGPU entry");
static_assert(wgentries::ModelEntryIndex('K', true, true) == 1, "Model K follows Model M");
static_assert(wgentries::ModelEntryIndex('L', true, true) == 2, "Model L follows Model K");
static_assert(wgentries::ModelEntryIndex('L', false, true) == 1, "without Model K, Model L follows Model M");
static_assert(wgentries::ModelEntryIndex('K', false, true) == -1, "a build without Model K lists none");

// WrongGpuHostSelectedV1's results (WrongGPU's include/wronggpu_host_event.h).
constexpr int32_t kRecorded = 1; // the selected model runs from now on: recreate, also for the entry already selected
constexpr int32_t kNothingChanged = 0;

// What the version box does after the player selected entry `selected` while `current` is selected, given WrongGPU's
// result (0 for a dll without the export).
struct VersionBoxAction
{
    bool setIndex = false; // store `selected` as the index
    bool recreate = false; // recreate the upscaler (the backend change)
    bool notSaved = false; // tell the player "Not saved, try again"
};

inline VersionBoxAction OnVersionSelected(int current, int selected, int32_t result)
{
    VersionBoxAction a;

    if (result < 0)
    {
        a.notSaved = true; // nothing changes: no new index, no recreate
        return a;
    }

    if (current != selected || result == kRecorded)
    {
        a.setIndex = true;
        a.recreate = true;
    }

    return a;
}

// Calls the dll's WrongGpuHostSelectedV1 when `resolve` finds it (a function returning the export's address or
// nullptr), else returns 0.
using PfnHostSelected = int32_t (*)(uint64_t versionId);

template <class Resolve> int32_t CallHostSelected(Resolve&& resolve, uint64_t versionId)
{
    const PfnHostSelected fn = resolve();
    return fn != nullptr ? fn(versionId) : kNothingChanged;
}

// The model of a listed version name ("WrongGPU Model K##3.1.0" -> 'K'), else 0.
inline char ModelOfName(const char* name)
{
    static const char kPrefix[] = "WrongGPU Model ";
    const size_t n = sizeof(kPrefix) - 1;

    if (name == nullptr || std::strncmp(name, kPrefix, n) != 0)
        return 0;

    const char m = name[n];
    return (m == 'M' || m == 'K' || m == 'L') && (name[n + 1] == 0 || name[n + 1] == '#') ? m : 0;
}

// True when WrongGPU's model entries in a version list (the names the dll returned, in its order: the index stored in
// OptiScaler.ini is a position in this list) are at the positions wg_model_entries.h defines. A list without WrongGPU's
// entries is true (not WrongGPU's dll).
inline bool ListOrderMatches(const std::vector<const char*>& names)
{
    bool hasK = false, hasL = false;

    for (auto name : names)
    {
        hasK = hasK || ModelOfName(name) == 'K';
        hasL = hasL || ModelOfName(name) == 'L';
    }

    for (size_t i = 0; i < names.size(); i++)
    {
        const char m = ModelOfName(names[i]);

        if (m != 0 && wgentries::ModelEntryIndex(m, hasK, hasL) != static_cast<int>(i))
            return false;
    }

    return true;
}

} // namespace wronggpu_edition
