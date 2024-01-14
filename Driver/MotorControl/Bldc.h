#pragma once

#include "system_ch32v20x.h"

#include "stdint.h"

namespace ES::Driver::MotorControl::Bldc {
    
    /*enum class Step : uint8_t{
        ChAl = 1,
        ChBl,
        AhBl,
        AhCl,
        BhCl,
        BhAl
    };*/

    enum class Step : uint8_t{
        AhBl = 1,
        ChBl,
        ChAl,
        BhAl,
        BhCl,
        AhCl
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

    enum class TorqueDivider : uint16_t {
        VeryHigh = 16000, 
        High = 24000, 
        Medium = 32000, 
        Low = 48000, 
        VeryLow = 64000
    };
}