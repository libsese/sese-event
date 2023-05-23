#include "sese/event/KqueueEventLoop.h"

#include <unistd.h>
#include <sys/event.h>
#include <sys/socket.h>

#define MAX_EVENT_SIZE 64

bool sese::event::KqueueEventLoop::init() {
    kqueue = ::kqueue();
    if (-1 == kqueue) return false;
    if (0 >= listenFd) return true;

    this->listenEvent = new KqueueEvent;
    this->listenEvent->fd = listenFd;
    this->listenEvent->events = EVENT_NULL;
    this->listenEvent->data = nullptr;

    struct kevent kevent{};
    EV_SET(&kevent, listenFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, this->listenEvent);
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

sese::event::KqueueEventLoop::~KqueueEventLoop() {
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
    struct kevent events[MAX_EVENT_SIZE]{};
    struct timespec timeout{};

    while(!isShutdown) {
        auto numberOfFds = ::kevent(kqueue, nullptr, 0, events, MAX_EVENT_SIZE, &timeout);
        if (-1 == numberOfFds) continue;
        for (int i = 0; i < numberOfFds; ++i) {
            auto event = reinterpret_cast<BaseEvent *>(events[i].udata);
            if (events[i].ident == listenFd) {
                auto client = accept(listenFd, nullptr, nullptr);
                if (client == -1) continue;
                onAccept(client);
                continue;
            }

            if (events[i].filter == EVFILT_READ) {
                if (events[i].flags & EV_ERROR && event->events & EVENT_ERROR) {
                    onError(event);
                }

                if (events[i].flags & EV_EOF) {
                    onClose(event);
                    continue;
                }

                if (events[i].flags == EV_ERROR | events[i].flags == EV_EOF) continue;
                onRead(event);
            } else if (events[i].filter == EVFILT_WRITE) {
                if (events[i].flags & EV_ERROR && event->events & EVENT_ERROR) {
                    onError(event);
                    continue;
                }

                onWrite(reinterpret_cast<BaseEvent *>(events[i].udata));
            }
        }
    }
}

void sese::event::KqueueEventLoop::stop() {
    isShutdown = true;
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

bool sese::event::KqueueEventLoop::addNativeEvent(int fd, uint32_t ev, void *data) const {
    struct kevent kevent {};
    EV_SET(&kevent, fd, ev, EV_ADD | EV_ENABLE, 0, 0, data);
    return 0 == ::kevent(kqueue, &kevent, 1, nullptr, 0, nullptr);
}

bool sese::event::KqueueEventLoop::delNativeEvent(int fd, uint32_t ev, void *data) const {
    struct kevent kevent{};
    EV_SET(&kevent, fd, ev, EV_DELETE, 0, 0, data);
    return 0 == ::kevent(kqueue, &kevent, 1, nullptr, 0, nullptr);
}

sese::event::BaseEvent *sese::event::KqueueEventLoop::createEvent(int fd, unsigned int events, void *data) {
    auto event = new KqueueEvent;
    event->fd = fd;
    event->events = events;
    event->oldEvents = events;
    event->data = data;

    if (!(events & EVENT_READ && addNativeEvent(fd, EVFILT_READ, event)) ){
        delete event;
        return nullptr;
    }

    if (!(events & EVENT_WRITE && addNativeEvent(fd, EVFILT_WRITE, event))) {
        delete event;
        return nullptr;
    }

    return event;
}

void sese::event::KqueueEventLoop::freeEvent(sese::event::BaseEvent *event) {
    auto fd = event->fd;
    auto events = event->events;
    if (events & EVENT_READ) {
        delNativeEvent(fd, EVFILT_READ, event);
    }

    if (events & EVENT_WRITE) {
        delNativeEvent(fd, EVFILT_WRITE, event);
    }
}

bool sese::event::KqueueEventLoop::setEvent(sese::event::BaseEvent *event) {
    auto e = reinterpret_cast<KqueueEvent *>(event);
    bool rt1 = true;
    bool rt2 = true;

    if (e->oldEvents & EVENT_READ && !(e->events & EVENT_READ)) {
        // 原先存在的事件，现在需要删除
        rt1 = delNativeEvent(e->fd, EVFILT_READ, event);
    } else if (e->events & EVENT_READ) {
        // 原先存在的事件，需要重新启用
        // 原先不存在的事件，需要添加
        rt1 = addNativeEvent(e->fd, EVFILT_READ, event);
    }

    if (e->oldEvents & EVENT_WRITE && !(e->events & EVENT_WRITE)) {
        // 原先存在的事件，现在需要删除
        rt2 = delNativeEvent(e->fd, EVFILT_WRITE, event);
    } else if (e->events & EVENT_WRITE) {
        // 原先存在的事件，需要重新启用
        // 原先不存在的事件，需要添加
        rt2 = addNativeEvent(e->fd, EVFILT_WRITE, event);
    }

    e->oldEvents = e->events;
    return rt1 && rt2;
}