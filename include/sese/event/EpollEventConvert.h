#pragma once

#include "sese/event/BaseEventConvert.h"

namespace sese {
    namespace event {
        class EpollEventConvert : public BaseEventConvert {
        public:
            short fromNativeEvent(uint32_t event) override;

            uint32_t toNativeEvent(short event) override;
        };
    }
}