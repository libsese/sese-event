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
    epoll_event event{};
    event.events = EPOLLIN | EPOLLERR;
    event.data.fd = listenFd;
    if (-1 == epoll_ctl(epoll, EPOLL_CTL_ADD, listenFd, &event)) return false;
    return true;
}

sese::event::EpollEventLoop::~EpollEventLoop() {
    if (!isShutdown) {
        isShutdown = true;
    }
    close(epoll);
}

void sese::event::EpollEventLoop::loop() {
    epoll_event events[MAX_EVENT_SIZE]{};

    while (!isShutdown) {
        int numberOfFds = epoll_wait(epoll, events, MAX_EVENT_SIZE, 0);
        if (-1 == numberOfFds) {
            continue;
        }

        for (int i = 0; i < numberOfFds; ++i) {
            auto fd = events[i].data.fd;
            if (fd == listenFd) {
                auto client = accept(fd, nullptr, nullptr);
                if (-1 == client) continue;
                onAccept(client);
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
    epoll_ctl(epoll, EPOLL_CTL_ADD, fd, &epollEvent);

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