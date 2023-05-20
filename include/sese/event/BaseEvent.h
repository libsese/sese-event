#pragma once

#include <cstdint>

namespace sese {
    namespace event {
        class BaseEvent;
    }
}

class sese::event::BaseEvent {
public:
    virtual bool init() = 0;

    virtual ~BaseEvent() = default;

    virtual void dispatch() = 0;

    virtual void stop() = 0;

    virtual void onAccept(int fd, short events) = 0;

    virtual void onRead(int fd, short events) = 0;

    virtual void onWrite(int fd, short events) = 0;

    virtual void onError(int fd, short events) = 0;

    virtual void setEvent(int fd, short events) = 0;
};