#include "sese/event/EpollEventConvert.h"

#include <sys/epoll.h>

short sese::event::EpollEventConvert::fromNativeEvent(uint32_t event) {
    short result = 0;
    if (event & EPOLLIN) {
        result |= EVENT_READ;
    }
    if (event & EPOLLOUT) {
        result |= EVENT_WRITE;
    }
    if (event & EPOLLERR) {
        result |= EVENT_ERROR;
    }
    return result;
}

uint32_t sese::event::EpollEventConvert::toNativeEvent(short event) {
    uint32_t result = 0;
    if (event & EVENT_READ) {
        result |= EPOLLIN;
    }
    if (event & EVENT_WRITE) {
        result |= EPOLLOUT;
    }
    if (event & EVENT_ERROR) {
        result |= EPOLLERR;
    }
    return result;
}
