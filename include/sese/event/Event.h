#pragma once

#if defined(__linux__)

#include "sese/event/EpollEvent.h"
#include "sese/event/EpollEventConvert.h"

namespace sese {
    namespace event {
        using Event = EpollEvent;
        using EventConvert = EpollEventConvert;
    }
}
#endif

