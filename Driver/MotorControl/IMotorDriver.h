#pragma once

#include "Bldc.h"

namespace ES::Driver::MotorControl {

    class IMotorDriver {
    public:
        virtual void commutate(Bldc::MotorConfiguration highNode, Bldc::MotorConfiguration lowNode) = 0;
        virtual void deCommutate(Bldc::MotorConfiguration node) = 0;
        virtual void init() = 0;
        virtual void deInit() = 0;
        virtual bool setDuty(uint32_t freqHz, float duty) = 0;
    };
}