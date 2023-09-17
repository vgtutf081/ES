#pragma once

namespace ES::Driver::MotorControl::Bldc {

    enum Phases {
        A,
        B,
        C
    };

    enum BemfEdge {
        Rising,
        Falling
    };

    struct MotorConfiguration {
    public:
        uint8_t N;
        uint8_t P;
    };

}