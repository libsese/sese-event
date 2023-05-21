#include "sese/event/WSAEventLoop.h"

#include <Winsock2.h>

#define MAX_EVENT_SIZE 64

bool sese::event::WSAEventLoop::init() {
    wsaEvent = WSACreateEvent();
    if (WSA_INVALID_EVENT == wsaEvent) return false;
    if (0 >= listenFd) return true;

    if (WSAEventSelect(listenFd, wsaEvent, FD_ACCEPT)) {
        WSACloseEvent(wsaEvent);
        wsaEvent = nullptr;
        return false;
    }

    this->listenEvent = new WSAEvent;
    this->listenEvent->events = EVENT_ERROR;
    this->listenEvent->fd = listenFd;
    this->listenEvent->wsaEvent = wsaEvent;

    return true;
}

sese::event::WSAEventLoop::~WSAEventLoop() {
    if (wsaEvent) {
        WSACloseEvent(wsaEvent);
        wsaEvent = nullptr;
    }

    if (listenEvent) {
        delete listenEvent;
        listenEvent = nullptr;
    }
}

void sese::event::WSAEventLoop::loop() {
    size_t numbers = 0;
    SOCKET sockets[MAX_EVENT_SIZE]{};
    HANDLE wsaEvents[MAX_EVENT_SIZE]{};
    WSAEvent *events[MAX_EVENT_SIZE]{}; // 此处生命周期应由用户复杂

    if (listenFd > 0) {
        sockets[0] = listenFd;
        wsaEvents[0] = listenEvent->wsaEvent;
        events[0] = listenEvent;
        numbers += 1;
    }

    while (!isShutdown) {
        DWORD nIndex = WSAWaitForMultipleEvents(numbers, wsaEvents, FALSE, 1000, FALSE);
        if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT) continue;

        nIndex -= WSA_WAIT_EVENT_0;
        for (DWORD i = nIndex; i < numbers; ++i) {
            nIndex = ::WSAWaitForMultipleEvents(1, &wsaEvents[i], TRUE, 1000, FALSE);
            if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT) continue;

            WSANETWORKEVENTS enumEvent;
            WSAEnumNetworkEvents(sockets[i], wsaEvents[i], &enumEvent);
            if (enumEvent.lNetworkEvents & FD_ACCEPT) {
                if (enumEvent.iErrorCode[FD_ACCEPT_BIT] == 0) {
                    SOCKET client = accept(sockets[i], nullptr, nullptr);
                    if (-1 != client) {
                        onAccept((int) client);
                    }
                } else if (enumEvent.iErrorCode[FD_ACCEPT_BIT] != 0 && events[i]->events & EVENT_ERROR) {
                    onError(events[i]);
                }
            } else if (enumEvent.lNetworkEvents & FD_READ) {
                if (enumEvent.iErrorCode[FD_READ_BIT] == 0) {
                    onRead(events[i]);
                } else if (enumEvent.iErrorCode[FD_READ_BIT] != 0 && events[i]->events & EVENT_ERROR) {
                    onError(events[i]);
                }
            } else if (enumEvent.lNetworkEvents & FD_WRITE) {
                if (enumEvent.iErrorCode[FD_WRITE_BIT] == 0) {
                    onWrite(events[i]);
                } else if (enumEvent.iErrorCode[FD_WRITE_BIT] != 0 && events[i]->events & EVENT_ERROR) {
                    onError(events[i]);
                }
            } else if (enumEvent.lNetworkEvents & FD_CLOSE) {
                WSACloseEvent(wsaEvents[i]);
                memmove(&sockets[i], &sockets[i], (numbers - i - 1) * sizeof(SOCKET));
                memmove(&wsaEvents[i], &wsaEvents[i], (numbers - i - 1) * sizeof(HANDLE));
                memmove(&events[i], &events[i], (numbers - i - 1) * sizeof(SOCKET));
                numbers -= 1;
            }
        }
    }
}

void sese::event::WSAEventLoop::stop() {
    isShutdown = true;
}

void sese::event::WSAEventLoop::onAccept(int fd) {

}

void sese::event::WSAEventLoop::onRead(sese::event::BaseEvent *event) {

}

void sese::event::WSAEventLoop::onWrite(sese::event::BaseEvent *event) {

}

void sese::event::WSAEventLoop::onError(sese::event::BaseEvent *event) {

}

sese::event::BaseEvent *sese::event::WSAEventLoop::createEvent(int fd, unsigned int events, void *data) {
    WSAEVENT _wsaEvent = WSACreateEvent();
    if (WSAEventSelect(fd, _wsaEvent, convert.toNativeEvent(events) | FD_CLOSE)) {
        WSACloseEvent(_wsaEvent);
        return nullptr;
    }

    auto event = new WSAEvent;
    event->fd = fd;
    event->events = events;
    event->data = data;
    event->wsaEvent = _wsaEvent;
    return event;
}

void sese::event::WSAEventLoop::freeEvent(sese::event::BaseEvent *event) {
    delete event;
}

bool sese::event::WSAEventLoop::setEvent(sese::event::BaseEvent *event) {
    auto ev = reinterpret_cast<WSAEvent *>(event);
    return 0 == WSAEventSelect(ev->fd, ev->wsaEvent, convert.toNativeEvent(ev->events) | FD_CLOSE);
}

void sese::event::WSAEventLoop::setListenFd(int fd) {
    this->listenFd = fd;
}
