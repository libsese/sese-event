#include "sese/event/KqueueEventConvert.h"

#include <sys/event.h>

unsigned int sese::event::KqueueEventConvert::fromNativeEvent(int event) {
    unsigned int result = 0;
    if (event & EVFILT_READ) {
        result |= EVENT_READ;
    }
    if (event & EVFILT_WRITE) {
        result |= EVENT_WRITE;
    }
    return result;
}

int sese::event::KqueueEventConvert::toNativeEvent(unsigned int event) {
    int result = 0;
    if (event & EVENT_READ) {
        result |= EVFILT_READ;
    }
    if (event & EVENT_WRITE) {
        result |= EVENT_WRITE;
    }
    return result;
}