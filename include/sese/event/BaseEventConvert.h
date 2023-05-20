#pragma once

#include <cstdint>

#define EVENT_READ 0x1
#define EVENT_WRITE 0x2
#define EVENT_ERROR 0x4

namespace sese {
    namespace event {
        class BaseEventConvert;
    }
}

class sese::event::BaseEventConvert {
public:
    virtual ~BaseEventConvert() = default;

    virtual short fromNativeEvent(uint32_t event) = 0;

    virtual uint32_t toNativeEvent(short event) = 0;
};