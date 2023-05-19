#include "sese/event/EpollEvent.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_EVENT_SIZE 64

bool sese::event::EpollEvent::init() {
    epoll = epoll_create1(0);
    if (-1 == epoll) return false;
    epoll_event event{};
    event.events = EPOLLIN | EPOLLERR;
    event.data.fd = listenFd;
    if (-1 == epoll_ctl(epoll, EPOLL_CTL_ADD, listenFd, &event)) return false;
    return true;
}

sese::event::EpollEvent::~EpollEvent() {
    if (!isShutdown) {
        isShutdown = true;
    }
    close(epoll);
}

void sese::event::EpollEvent::dispatch() {
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
                onAccept(client, events[i].events);
            } else if (events[i].events & EPOLLIN) {
                onRead(fd, events[i].events);
            } else if (events[i].events & EPOLLOUT) {
                onWrite(fd, events[i].events);
            } else if (events[i].events & EPOLLERR) {
                onError(fd, events[i].events);
            }
        }
    }
}

void sese::event::EpollEvent::stop() {
    isShutdown = true;
}

void sese::event::EpollEvent::onAccept(int fd, short events) {

}

void sese::event::EpollEvent::onRead(int fd, short events) {

}

void sese::event::EpollEvent::onWrite(int fd, short events) {

}

void sese::event::EpollEvent::onError(int fd, short events) {

}