#pragma once

#include "debug.h"
#include <functional>

namespace ES::Driver::Systick {

    class SystickCh32 {
    public:
        SystickCh32(uint32_t hz,const std::function<void()>& systickCallback, uint8_t interruptPriority = 0xE0);

        ~SystickCh32();

        void systickRoutine() {
            SysTick->SR = 0;
            _systickCallback();
        }
    private:
        static bool _systickBusy;
        std::function<void()> _systickCallback;
    };
}