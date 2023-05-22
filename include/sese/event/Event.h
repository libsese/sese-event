#pragma once

#if defined(__linux__)

#include "sese/event/EpollEvent.h"
#include "sese/event/EpollEventLoop.h"
#include "sese/event/EpollEventConvert.h"

namespace sese {
    namespace event {
        using Event = BaseEvent;
        using EventLoop = EpollEventLoop;
        using EventConvert = EpollEventConvert;
    }
}

#endif

#if defined(WIN32)

#include "sese/event/WSAEvent.h"
#include "sese/event/WSAEventLoop.h"
#include "sese/event/WSAEventConvert.h"

namespace sese {
    namespace event {
        using Event = BaseEvent;
        using EventLoop = WSAEventLoop;
        using EventConvert = WSAEventConvert;
    }
}

#endif

