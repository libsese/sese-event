#pragma once

#include "sese/event/KqueueEvent.h"
#include "sese/event/BaseEventLoop.h"
#include "sese/event/KqueueEventConvert.h"

#include <atomic>

namespace sese {
    namespace event {
        class KqueueEventLoop;
    }
}

class sese::event::KqueueEventLoop : public sese::event::BaseEventLoop {
public:
    bool init() override;

    ~KqueueEventLoop() override;

    void loop() override;

    void stop() override;

    void onAccept(int fd) override;

    void onRead(BaseEvent *event) override;

    void onWrite(BaseEvent *event) override;

    void onError(BaseEvent *event) override;

    void onClose(BaseEvent *event) override;

    BaseEvent *createEvent(int fd, unsigned int events, void *data) override;

    void freeEvent(BaseEvent *event) override;

    bool setEvent(BaseEvent *event) override;

    void setListenFd(int fd) override { listenFd = fd; }

protected:
    int listenFd{-1};
    BaseEvent *listenEvent{nullptr};

    int kqueue{-1};
    std::atomic_bool isShutdown{false};
    KqueueEventConvert convert;
};