#pragma once

namespace ES::Driver::I2S {
    
    enum class SampleRate : uint32_t {
        Hz16000 = 16000,
        Hz32000 = 32000,
        Hz48000 = 48000,
        Hz96000 = 96000
    };

    enum Channel {
        Right,
        Left,
        Both
    };

    enum SampleWidth {
        Bit8,
        Bit16,
        Bit24
    };
}