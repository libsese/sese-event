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

void threadProc(sese::event::Event *event) {
    event->dispatch();
}

TEST(TestEvent, Linux) {
    class MyEvent : public sese::event::Event {
    public:
        void onAccept(int fd, short events) override {
            if (0 == setNonblocking(fd)) {
                this->setEvent(fd, EVENT_READ | EVENT_WRITE);
            } else {
                close(fd);
            }
        }

        void onRead(int fd, short events) override {
            char buffer[1024]{};
            while (true) {
                auto len = read(fd, buffer, 1024);
                // printf("recv %d bytes\n", (int) len);
                if (len == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        this->setEvent(fd, events);
                    } else {
                        close(fd);
                    }
                    break;
                } else {
                    recv += len;
                }
            }
        }

        void onWrite(int fd, short events) override {
            printf("on write\n");
        }

        [[nodiscard]] size_t getRecv() const { return recv; }

    protected:
        size_t recv = 0;
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

    char buffer[1024];
    size_t send = 0;
    for (int i = 0; i < 5; ++i) {
        auto len = write(client, buffer, 1024);
        // printf("send %d bytes\n", (int) len);
        send += len;
    }

    std::this_thread::sleep_for(3s);
    EXPECT_EQ(event.getRecv(), send);
    event.stop();
    th.join();

    close(client);
    close(listenSocket);
}

TEST(TestEventConvert, Linux) {
    sese::event::BaseEventConvert *convert = new sese::event::EventConvert();
    {
        uint32_t ev1 = EPOLLIN | EPOLLOUT;
        short ev2 = convert->fromNativeEvent(ev1);
        ASSERT_EQ(ev2, EVENT_READ | EVENT_WRITE);
    }
    {
        short ev1 = EVENT_ERROR | EVENT_READ;
        uint32_t ev2 = convert->toNativeEvent(ev1);
        ASSERT_EQ(ev2, EPOLLERR | EPOLLIN);
    }
    delete convert;
}