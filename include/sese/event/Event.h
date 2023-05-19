#pragma once

#if defined(__linux__)
#include "sese/event/EpollEvent.h"
namespace sese {
    namespace event {
        using Event = EpollEvent;
    }
}
#endif

