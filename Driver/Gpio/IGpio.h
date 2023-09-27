#pragma once

#include "Gpio.h"
#include "ITimer.h"

namespace ES::Driver::Gpio {
    
    struct IGpio {
        
    public:
        virtual void set() = 0;

        virtual void reset() = 0;

        virtual void toggle() = 0;

        virtual bool read() = 0;

        virtual void disable() = 0;

        virtual uint32_t getPin() = 0;

        virtual void setMode(PinMode mode, DriveMode driveMode, PullMode pullMode) = 0;

        inline void configureOutput(DriveMode driveMode = DriveMode::PushPull, PullMode pullMode = PullMode::None) {
            setMode(PinMode::Output, driveMode, pullMode);
        }

        inline void configureInput(PullMode pullMode = PullMode::None) {
            setMode(PinMode::Input, DriveMode::None, pullMode);
        }

    };
}