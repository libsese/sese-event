#include "sese/event/BaseEventConvert.h"
#include "sese/event/Event.h"
#include "gtest/gtest.h"

#include <random>
#include <sys/epoll.h>
#include <thread>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std::chrono_literals;

int setNonblocking(int fd) {
    auto option = fcntl(fd, F_GETFL);
    if (option != -1) {
        return fcntl(fd, F_SETFL, option | O_NONBLOCK);
    } else {
        return -1;
    }
}

void makeRandomPortAddress(sockaddr_in &in) {
    srandom((unsigned) time(nullptr));
    auto port = random() % (65535 - 1024) + 1024;
    inet_pton(AF_INET, "127.0.0.1", &in.sin_addr);
    in.sin_family = AF_INET;
    in.sin_port = htons(port);
    printf("select port %d\n", (int) port);
}

void threadProc(sese::event::EventLoop *event) {
    event->loop();
}

TEST(TestEvent, Linux) {
    class MyEvent : public sese::event::EventLoop {
    public:
        void onAccept(int fd) override {
            if (0 == setNonblocking(fd)) {
                this->createEvent(fd, EVENT_READ | EVENT_WRITE, nullptr);
            } else {
                close(fd);
            }
        }

        void onRead(sese::event::Event *event) override {
            char buffer[1024]{};
            while (true) {
                auto len = read(event->fd, buffer, 1024);
                // printf("recv %d bytes\n", (int) len);
                if (len == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        this->setEvent(event);
                    } else {
                        close(event->fd);
                        this->freeEvent(event);
                    }
                    break;
                } else {
                    recv += len;
                }
            }
            if (recv == 1024 * 5) {
                close(event->fd);
                this->freeEvent(event);
            }
        }

        void onWrite(sese::event::Event *event) override {
            printf("on write\n");
            event->events &= ~EVENT_WRITE;
            this->setEvent(event);
        }

        [[nodiscard]] size_t getRecv() const { return recv; }

    protected:
        std::atomic_long recv{0};
    };

    sockaddr_in address{};
    makeRandomPortAddress(address);

    auto listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_EQ(setNonblocking(listenSocket), 0);
    ASSERT_EQ(bind(listenSocket, (sockaddr *) &address, sizeof(address)), 0);
    listen(listenSocket, 255);

    MyEvent event;
    event.setListenFd(listenSocket);
    ASSERT_TRUE(event.init());
    auto th = std::thread(threadProc, &event);

    auto client = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(client, (sockaddr *) &address, sizeof(address)) != 0) {
        event.stop();
        th.join();
        FAIL();
    }

    char buffer[1024]{};
    size_t send = 0;
    for (int i = 0; i < 5; ++i) {
        auto len = write(client, buffer, 1024);
        // printf("send %d bytes\n", (int) len);
        send += len;
    }

    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(event.getRecv(), send);
    event.stop();
    th.join();

    close(client);
    close(listenSocket);
}

TEST(TestEventConvert, Linux) {
    sese::event::BaseEventConvert *convert = new sese::event::EventConvert();
    {
        int ev1 = EPOLLIN | EPOLLOUT;
        unsigned ev2 = convert->fromNativeEvent(ev1);
        ASSERT_EQ(ev2, EVENT_READ | EVENT_WRITE);
    }
    {
        unsigned ev1 = EVENT_ERROR | EVENT_READ;
        int ev2 = convert->toNativeEvent(ev1);
        ASSERT_EQ(ev2, EPOLLERR | EPOLLIN);
    }
    delete convert;
}