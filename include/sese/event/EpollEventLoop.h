#pragma once

#include "sese/event/EpollEvent.h"
#include "sese/event/BaseEventLoop.h"
#include "sese/event/EpollEventConvert.h"

#include <atomic>

namespace sese {
    namespace event {
        class EpollEventLoop;
    }
}

class sese::event::EpollEventLoop : public sese::event::BaseEventLoop {
public:
    bool init() override;

    ~EpollEventLoop() override;

    void loop() override;

    void stop() override;

    void onAccept(int fd) override;

    void onRead(BaseEvent *event) override;

    void onWrite(BaseEvent *event) override;

    void onError(BaseEvent *event) override;

    BaseEvent *createEvent(int fd, unsigned int events, void *data) override;

    void freeEvent(BaseEvent *event) override;

    bool setEvent(BaseEvent *event) override;

public:
    void setListenFd(int fd) { listenFd = fd; }

protected:
    int listenFd{-1};
    int epoll{-1};
    std::atomic_bool isShutdown{false};
    EpollEventConvert convert;
};