#pragma once

#include "sese/event/BaseEvent.h"

#include <atomic>

namespace sese {
    namespace event {
        class EpollEvent;
    }
}

class sese::event::EpollEvent : public sese::event::BaseEvent {
public:
    explicit EpollEvent(int listenFd);

    ~EpollEvent() override;

    void dispatch() override;

    void stop() override;

    void onAccept(int fd) override;

    void onRead(int fd) override;

    void onWrite(int fd) override;

    void onError(int fd) override;

protected:
    int listenFd{-1};
    int epoll{-1};
    std::atomic_bool isShutdown{false};
};