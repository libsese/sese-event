#include "sese/event/EpollEvent.h"
#include "sese/event/BaseEvent.h"

#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENT_SIZE 64

sese::event::EpollEvent::EpollEvent(int listenFd) : BaseEvent() {
    this->listenFd = listenFd;
    epoll = epoll_create1(0);
    epoll_event event{};
    event.events = EPOLLIN | EPOLLERR;
    event.data.fd = listenFd;
    epoll_ctl(epoll, EPOLL_CTL_ADD, listenFd, &event);
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

        for (int i =0; i < numberOfFds; ++i) {
            auto fd = events[i].data.fd;
            if (fd == listenFd) {
                onAccept(fd);
                continue;
            } else if (events[i].events & EPOLLIN) {
                onRead(fd);
                continue;
            } else if (events[i].events & EPOLLOUT) {
                onWrite(fd);
                continue;
            } else if (events[i].events & EPOLLERR) {
                onError(fd);
                continue;
            }
        }
    }
}

void sese::event::EpollEvent::stop() {
    isShutdown = true;
}

void sese::event::EpollEvent::onAccept(int fd) {

}

void sese::event::EpollEvent::onRead(int fd) {

}

void sese::event::EpollEvent::onWrite(int fd) {

}

void sese::event::EpollEvent::onError(int fd) {

}
