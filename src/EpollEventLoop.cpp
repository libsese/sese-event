#include "sese/event/EpollEventLoop.h"
#include "sese/event/BaseEvent.h"
#include "sese/event/EpollEvent.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>

#define MAX_EVENT_SIZE 64

bool sese::event::EpollEventLoop::init() {
    epoll = epoll_create1(0);
    if (-1 == epoll) return false;
    if (0 >= listenFd) return true;

    this->listenEvent = this->createEvent(listenFd, EVENT_READ | EVENT_ERROR, nullptr);
    // 通常不会失败
    if (!this->listenEvent) return false;
    return true;
}

sese::event::EpollEventLoop::~EpollEventLoop() {
    if (-1 != epoll) {
        close(epoll);
    }

    if (listenEvent) {
        delete listenEvent;
        listenEvent = nullptr;
    }
}

void sese::event::EpollEventLoop::loop() {
    epoll_event events[MAX_EVENT_SIZE]{};

    while (!isShutdown) {
        int numberOfFds = epoll_wait(epoll, events, MAX_EVENT_SIZE, 0);
        if (-1 == numberOfFds) {
            continue;
        }

        for (int i = 0; i < numberOfFds; ++i) {
            auto event = (BaseEvent *) events[i].data.ptr;
            auto fd = event->fd;
            if (fd == listenFd) {
                auto client = accept(fd, nullptr, nullptr);
                if (-1 != client) {
                    onAccept(client);
                } else {
                    continue;
                }
            } else if (events[i].events & EPOLLIN) {
                onRead((BaseEvent *) events[i].data.ptr);
            } else if (events[i].events & EPOLLOUT) {
                onWrite((BaseEvent *) events[i].data.ptr);
            } else if (events[i].events & EPOLLERR) {
                onError((BaseEvent *) events[i].data.ptr);
            }
        }
    }
}

void sese::event::EpollEventLoop::stop() {
    isShutdown = true;
}

void sese::event::EpollEventLoop::onAccept(int fd) {

}

void sese::event::EpollEventLoop::onRead(BaseEvent *event) {

}

void sese::event::EpollEventLoop::onWrite(BaseEvent *event) {

}

void sese::event::EpollEventLoop::onError(BaseEvent *event) {

}

sese::event::BaseEvent *sese::event::EpollEventLoop::createEvent(int fd, unsigned int events, void *data) {
    auto event = new EpollEvent;
    event->fd = fd;
    event->events = events;
    event->data = data;

    epoll_event epollEvent{};
    epollEvent.events = convert.toNativeEvent(events);
    epollEvent.data.ptr = event;
    if (-1 == epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &epollEvent)) {
        delete event;
        return nullptr;
    }

    return event;
}

void sese::event::EpollEventLoop::freeEvent(sese::event::BaseEvent *event) {
    delete event;
}

bool sese::event::EpollEventLoop::setEvent(sese::event::BaseEvent *event) {
    epoll_event epollEvent{};
    epollEvent.events = convert.toNativeEvent(event->events);
    epollEvent.data.ptr = event;

    auto result = epoll_ctl(epoll, EPOLL_CTL_MOD, event->fd, &epollEvent);
    return (result == 0 || (result == -1 && errno == EEXIST));
}