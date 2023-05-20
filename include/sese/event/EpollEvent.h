#pragma once

#include "sese/event/BaseEvent.h"
#include "sese/event/EpollEventConvert.h"

#include <atomic>

namespace sese {
    namespace event {
        class EpollEvent;
    }
}

class sese::event::EpollEvent : public sese::event::BaseEvent {
public:
    bool init() override;

    ~EpollEvent() override;

    void dispatch() override;

    void stop() override;

    void onAccept(int fd, short events) override;

    void onRead(int fd, short events) override;

    void onWrite(int fd, short events) override;

    void onError(int fd, short events) override;

    void setEvent(int fd, short events) override;

public:
    void setListenFd(int fd) { listenFd = fd; }

protected:
    int listenFd{-1};
    int epoll{-1};
    std::atomic_bool isShutdown{false};
    EpollEventConvert convert;
};