#include "sese/event/Event.h"
#include "gtest/gtest.h"

#include <unistd.h>
#include <thread>
#include <random>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <netinet/in.h>

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
            close(fd);
        }
    }

    void onRead(sese::event::Event *event) override {
        char buffer[1024]{};
        while (recv != 1024 * 5) {
            auto len = read(event->fd, buffer, 1024);
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
        close(event->fd);
        this->freeEvent(event);
    }

    void onWrite(sese::event::Event *event) override {
        printf("on write\n");
        event->events &= ~EVENT_WRITE;
        this->setEvent(event);
    }

    void onClose(sese::event::Event *event) override {
        this->freeEvent(event);
        SUCCEED() << "succeed auto to close";
    }

    [[nodiscard]] size_t getRecv() const { return recv; }

protected:
    std::atomic_long recv{0};
};

TEST(TestEvent, DarwinRead) {
    sockaddr_in address{};
    makeRandomPortAddress(address);

    auto listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_EQ(setNonblocking(listenSocket), 0);
    ASSERT_EQ(bind(listenSocket, (sockaddr *) &address, sizeof(address)), 0);
    listen(listenSocket, 255);

    MyEvent event;
    event.setListenFd(listenSocket);
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

TEST(TestEvent, AutoClose) {
    sockaddr_in address{};
    makeRandomPortAddress(address);

    auto listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_EQ(setNonblocking(listenSocket), 0);
    ASSERT_EQ(bind(listenSocket, (sockaddr *) &address, sizeof(address)), 0);
    listen(listenSocket, 255);

    MyEvent event;
    event.setListenFd(listenSocket);
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
        auto len = write(client, buffer, 1024);
        // printf("send %d bytes\n", (int) len);
        send += len;
    }

    std::this_thread::sleep_for(100ms);
    shutdown(client, SHUT_RDWR);
    close(client);
    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(event.getRecv(), send);
    event.stop();
    th.join();

    close(listenSocket);
}