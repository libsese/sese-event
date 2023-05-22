#include "sese/event/Event.h"
#include "gtest/gtest.h"

#include <thread>
#include <chrono>
#include <Winsock2.h>
#include <WS2tcpip.h>

using namespace std::chrono_literals;

int setNonblocking(int fd) {
    unsigned long ul = 1;
    return ioctlsocket(fd, FIONBIO, &ul);
}

void makeRandomPortAddress(sockaddr_in &in) {
    std::srand((unsigned int) std::time(nullptr));       // NOLINT
    auto port = std::rand() % (65535 - 1024) + 1024; // NOLINT
    inet_pton(AF_INET, "127.0.0.1", &in.sin_addr);
    in.sin_family = AF_INET;
    in.sin_port = htons(port);
    printf("select port %d\n", (int) port);
}

TEST(TestEvent, WindowsRead) {
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
            while (true) {
                auto len = ::recv(event->fd, buffer, 1024, 0);
                // printf("recv %d bytes\n", (int) len);
                if (len == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        this->setEvent(event);
                    } else {
                        shutdown(event->fd, SD_BOTH);
                        closesocket(event->fd);
                        this->freeEvent(event);
                    }
                    break;
                } else {
                    recv += len;
                }
            }
        }

        void onWrite(sese::event::BaseEvent *event) override {
            printf("on write\n");
            event->events &= ~EVENT_WRITE;
            // this->setEvent(event);
        }

        [[nodiscard]] size_t getRecv() const { return recv; }

    protected:
        std::atomic_long recv{0};
    };

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

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();

    WSACleanup();
    return result;
}