#pragma once

namespace ES::Driver::MotorControl::Bldc {
    
    enum class Step : uint8_t{
        ChAl = 1,
        ChBl,
        AhBl,
        AhCl,
        BhCl,
        BhAl
    };

    enum MotorPhase {
        A,
        B,
        C
    };

    enum BemfEdge {
        Rising,
        Falling
    };

    enum class LiPoCells : uint8_t {
        twoS = 2,
        fourS = 4,
        sixS = 6
    };

    static constexpr uint16_t freq16Khz = 144000000 / 16000;
    static constexpr uint16_t freq32Khz = 144000000 / 32000;
    static constexpr uint16_t freq48Khz = 144000000 / 48000;

    enum class Torque : uint16_t {
        High = freq16Khz, 
        Medium = freq32Khz, 
        Low = freq48Khz, 
    };
}