#pragma once

#include <windows.h>

namespace keyhook {
    inline HHOOK hook = NULL;

    void RegisterKeyhook() noexcept;
    void RemoveKeyhook() noexcept;
    void StartKeyhook() noexcept;
}

