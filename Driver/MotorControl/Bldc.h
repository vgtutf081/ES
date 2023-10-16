#pragma once

namespace ES::Driver::MotorControl::Bldc {
    
    enum Step : uint8_t{
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

    enum LiPoCells : uint8_t {
        twoS = 2,
        fourS = 4,
        sixS = 6
    };
}