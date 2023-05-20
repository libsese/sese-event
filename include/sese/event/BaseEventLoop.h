#pragma once

#include "sese/event/EpollEvent.h"

namespace sese {
    namespace event {
        class BaseEventLoop;
    }
}

class sese::event::BaseEventLoop {
public:
    virtual bool init() = 0;

    virtual ~BaseEventLoop() = default;

    virtual void loop() = 0;

    virtual void stop() = 0;

    virtual void onAccept(int fd) = 0;

    virtual void onRead(BaseEvent *event) = 0;

    virtual void onWrite(BaseEvent *event) = 0;

    virtual void onError(BaseEvent *event) = 0;

    virtual BaseEvent *createEvent(int fd, unsigned int events, void *data) = 0;

    virtual void freeEvent(BaseEvent *event) = 0;

    virtual bool setEvent(BaseEvent *event) = 0;

    virtual void setListenFd(int fd) = 0;
};