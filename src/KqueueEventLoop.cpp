#include "sese/event/KqueueEventLoop.h"

#include <unistd.h>
#include <sys/event.h>

bool sese::event::KqueueEventLoop::init() {
    kqueue = ::kqueue();
    if (-1 == kqueue) return false;
    if (0 >= listenFd) return true;

    this->listenEvent = new KqueueEvent;
    this->listenEvent->fd = listenFd;
    this->listenEvent->events = EVENT_NULL;
    this->listenEvent->data = nullptr;

    struct kevent kevent{};
    EV_SET(&kevent, listenFd, EVENT_READ, EV_ADD | EV_ENABLE, 0, 0, this->listenEvent);
    // 通常不会失败
    if (-1 == ::kevent(kqueue, &kevent, 1, nullptr, 0, nullptr)) {
        delete this->listenEvent;
        this->listenEvent = nullptr;

        close(this->kqueue);
        this->kqueue = -1;

        return false;
    }
    return true;
}

sese::event::KqueueEventLoop::~KqueueEventLoop() noexcept {
    if (-1 != kqueue) {
        close(kqueue);
        kqueue = -1;
    }

    if (listenEvent) {
        delete listenEvent;
        listenEvent = nullptr;
    }
}

void sese::event::KqueueEventLoop::loop() {

}

void sese::event::KqueueEventLoop::stop() {

}

void sese::event::KqueueEventLoop::onAccept(int fd) {

}

void sese::event::KqueueEventLoop::onRead(sese::event::BaseEvent *event) {

}

void sese::event::KqueueEventLoop::onWrite(sese::event::BaseEvent *event) {

}

void sese::event::KqueueEventLoop::onError(sese::event::BaseEvent *event) {

}

void sese::event::KqueueEventLoop::onClose(sese::event::BaseEvent *event) {

}

sese::event::BaseEvent *sese::event::KqueueEventLoop::createEvent(int fd, unsigned int events, void *data) {
    auto event = new KqueueEvent;
    event->fd = fd;
    event->events = events;
    event->data = data;

    struct kevent kevent{};
    EV_SET(&kevent,
            fd,
            convert.toNativeEvent(events),
            EV_ADD | EV_ONESHOT,
            0,
            0,
            event);
    if (-1 == ::kevent(kqueue, &kevent, 1, nullptr, 0, nullptr)) {
        delete event;
        return nullptr;
    } else {
        return event;
    }
}

void sese::event::KqueueEventLoop::freeEvent(sese::event::BaseEvent *event) {
    struct kevent kevent{};
    EV_SET(&kevent,
           event->fd,
           0,
           EV_DELETE,
           0,
           0,
           nullptr);
    ::kevent(kqueue, &kevent, 1, nullptr, 0, nullptr);
}

bool sese::event::KqueueEventLoop::setEvent(sese::event::BaseEvent *event) {
    struct kevent kevent{};
    EV_SET(&kevent, event->fd, convert.toNativeEvent(event->events), EV_ADD | EV_ONESHOT, 0, 0, event);
    return 0 == ::kevent(kqueue, &kevent, 1, nullptr, 0, nullptr);
}