#pragma once

#include "sese/event/WSAEvent.h"
#include "sese/event/BaseEventLoop.h"
#include "sese/event/WSAEventConvert.h"

#include <atomic>

namespace sese {
    namespace event {
        class WSAEventLoop;
    }
}

class sese::event::WSAEventLoop : public BaseEventLoop {
public:
    ~WSAEventLoop() override;

    bool init() override;

    void loop() override;

    void stop() override;

    void onAccept(int fd) override;

    void onRead(BaseEvent *event) override;

    void onWrite(BaseEvent *event) override;

    void onError(BaseEvent *event) override;

    BaseEvent *createEvent(int fd, unsigned int events, void *data) override;

    void freeEvent(BaseEvent *event) override;

    bool setEvent(BaseEvent *event) override;

    void setListenFd(int fd) override;

protected:
    int listenFd{-1};
    WSAEvent *listenEvent{nullptr};

    void *wsaEvent{nullptr};
    std::atomic_bool isShutdown{false};
    WSAEventConvert convert;
};