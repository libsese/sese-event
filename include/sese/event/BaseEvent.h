#pragma once

#include <cstdint>

namespace sese {
    namespace event {
        class BaseEvent;
    }
}

class sese::event::BaseEvent {
public:
    BaseEvent() = default;

    virtual ~BaseEvent() = default;

    virtual void dispatch() = 0;

    virtual void stop() = 0;

    virtual void onAccept(int fd) = 0;

    virtual void onRead(int fd) = 0;

    virtual void onWrite(int fd) = 0;

    virtual void onError(int fd) = 0;
};