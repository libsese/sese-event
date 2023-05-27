#include "sese/event/BaseEvent.h"
#include "sese/event/Event.h"
#include "gtest/gtest.h"

#include <thread>
#include <chrono>
#include <random>
#include <Winsock2.h>
#include <WS2tcpip.h>

using namespace std::chrono_literals;

int setNonblocking(int fd) {
    unsigned long ul = 1;
    return ioctlsocket(fd, FIONBIO, &ul);
}

void makeRandomPortAddress(sockaddr_in &in) {
    std::random_device device;
    auto engine = std::default_random_engine(device());
    std::uniform_int_distribution<int> dis(1025, 65535);
    auto port = dis(engine);

    inet_pton(AF_INET, "127.0.0.1", &in.sin_addr);
    in.sin_family = AF_INET;
    in.sin_port = htons(port);
    printf("select port %d\n", (int) port);
}

class MyEvent : public sese::event::EventLoop {
public:
    void onAccept(int fd) override {
        if (0 == setNonblocking(fd)) {
            this->createEvent(fd, EVENT_READ | EVENT_WRITE, nullptr);
        } else {
            closesocket(fd);
        }
    }

    void onRead(sese::event::BaseEvent *event) override {
        char buffer[1024]{};
        while (recv != 1024 * 5) {
            auto len = ::recv(event->fd, buffer, 1024, 0);
            // printf("recv %d bytes\n", (int) len);
            if (len == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    this->setEvent(event);
                } else if (errno == ENOTCONN) {
                    return;
                }
            } else if (len == 0) {
                return;
            } else {
                recv += len;
            }
        }
        closesocket(event->fd);
        this->freeEvent(event);
    }

    void onWrite(sese::event::BaseEvent *event) override {
        printf("on write\n");
        event->events &= ~EVENT_WRITE;
        this->setEvent(event);
    }

    void onClose(sese::event::BaseEvent *event) override {
        this->freeEvent(event);
        SUCCEED() << "succeed auto to close";
    }

    void loop () {
        while(run) {
            this->dispatch(1000);
        }
    }

    void stop() {
        run = false;
    }

    [[nodiscard]] size_t getRecv() const { return recv; }

protected:
    std::atomic_long recv{0};
    std::atomic_bool run{true};
};

TEST(TestEvent, WindowsRead) {
    sockaddr_in address{};
    makeRandomPortAddress(address);

    auto listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_EQ(setNonblocking((int) listenSocket), 0);
    ASSERT_EQ(bind(listenSocket, (sockaddr *) &address, sizeof(address)), 0);
    listen(listenSocket, 255);

    MyEvent event;
    event.setListenFd((int) listenSocket);
    ASSERT_TRUE(event.init());
    auto th = std::thread(std::bind(&MyEvent::loop, &event)); // NOLINT

    auto client = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(client, (sockaddr *) &address, sizeof(address)) != 0) {
        event.stop();
        th.join();
        FAIL();
    }

    char buffer[1024]{};
    size_t send = 0;
    for (int i = 0; i < 5; ++i) {
        auto len = ::send(client, buffer, 1024, 0);
        // printf("send %d bytes\n", (int) len);
        send += len;
    }

    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(event.getRecv(), send);
    event.stop();
    th.join();

    closesocket(listenSocket);
    closesocket(client);
}

TEST(TestEvent, AutoClose) {
    sockaddr_in address{};
    makeRandomPortAddress(address);

    auto listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_EQ(setNonblocking((int) listenSocket), 0);
    ASSERT_EQ(bind(listenSocket, (sockaddr *) &address, sizeof(address)), 0);
    listen(listenSocket, 255);

    MyEvent event;
    event.setListenFd((int) listenSocket);
    ASSERT_TRUE(event.init());
    auto th = std::thread(std::bind(&MyEvent::loop, &event)); // NOLINT

    auto client = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(client, (sockaddr *) &address, sizeof(address)) != 0) {
        event.stop();
        th.join();
        FAIL();
    }

    char buffer[1024]{};
    size_t send = 0;
    for (int i = 0; i < 4; ++i) {
        auto len = ::send(client, buffer, 1024, 0);
        // printf("send %d bytes\n", (int) len);
        send += len;
    }

    std::this_thread::sleep_for(100ms);
    shutdown(client, SD_BOTH);
    closesocket(listenSocket);
    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(event.getRecv(), send);
    event.stop();
    th.join();

    closesocket(client);
}

TEST(TestEventConvert, Windows) {
    sese::event::BaseEventConvert *convert = new sese::event::EventConvert();
    {
        int ev1 = FD_READ | FD_WRITE;
        unsigned ev2 = convert->fromNativeEvent(ev1);
        ASSERT_EQ(ev2, EVENT_READ | EVENT_WRITE);
    }
}

// 1000M
#define ALL_PACKAGES_SIZE (1024 * 1000 * 1000)
// 5K
#define PACKAGE_SIZE (1024 * 5)

class MyEvent1 : public sese::event::EventLoop {
public:
    void onAccept(int fd) override {
        if (0 == setNonblocking(fd)) {
            createEvent(fd, EVENT_WRITE, nullptr);
        } else {
            closesocket(fd);
        }
    }

    void onWrite(sese::event::BaseEvent *event) override {
        char buffer[PACKAGE_SIZE]{};
        while (send < ALL_PACKAGES_SIZE) {
            auto toWrite = ALL_PACKAGES_SIZE - send >= PACKAGE_SIZE ? PACKAGE_SIZE : ALL_PACKAGES_SIZE - send;
            auto l = ::send(event->fd, buffer, toWrite, 0);
            if (-1 == l) {
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                    SUCCEED() << "wait for next time";
                    this->setEvent(event);
                } else if (errno == ENOTCONN) {
                    FAIL();
                    return;
                }
            } else {
                send += l;
            }
        }
        closesocket(event->fd);
        freeEvent(event);
    }

    void loop() {
        while (run) {
            this->dispatch(1000);
        }
    }

    void stop() {
        run = false;
    }

    long getSend() const {
        return send;
    }

protected:
    std::atomic_long send{0};
    std::atomic_bool run{true};
};

TEST(TestEvent, WindowsWrite) {
    sockaddr_in address{};
    makeRandomPortAddress(address);

    auto listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_EQ(setNonblocking((int) listenSocket), 0);
    ASSERT_EQ(bind(listenSocket, (sockaddr *) &address, sizeof(address)), 0);
    listen(listenSocket, 255);

    MyEvent1 event;
    event.setListenFd((int) listenSocket);
    ASSERT_TRUE(event.init());
    auto th = std::thread(std::bind(&MyEvent1::loop, &event)); // NOLINT

    auto client = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(client, (sockaddr *) &address, sizeof(address)) != 0) {
        event.stop();
        th.join();
        FAIL();
    }

    char buffer[PACKAGE_SIZE];
    size_t recv = 0;
    while (recv < ALL_PACKAGES_SIZE) {
        auto toRead = ALL_PACKAGES_SIZE - recv >= PACKAGE_SIZE ? PACKAGE_SIZE : ALL_PACKAGES_SIZE - recv;
        auto l = ::recv(client, buffer, (int) toRead, 0);
        recv += l;
    }

    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(event.getSend(), recv);
    event.stop();
    th.join();

    closesocket(client);
    closesocket(listenSocket);
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();

    WSACleanup();
    return result;
}