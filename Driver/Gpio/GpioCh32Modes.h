#pragma once

#include <cstdint>

namespace ES::Driver::Gpio {
    enum class PullMode : uint32_t {
        None,
        Up,
        Down
    };

    enum class DriveMode : uint32_t {
        PushPull,
        OpenDrain,
        None
    };

    enum class PinMode : uint32_t {
        Input,
        Output,
        AnalogInput,
        Alternate
    };
    
    enum class InterruptMode : uint32_t {
        None,
        Rising,
        Falling,
        Both
    };
}