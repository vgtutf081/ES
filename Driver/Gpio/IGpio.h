#pragma once

#include "Gpio.h"

namespace ES::Driver::Gpio {
    
    class IGpio {
        
    public:
        virtual void set() = 0;

        virtual void reset() = 0;

        virtual void toggle() = 0;

        virtual bool read() const = 0;

        virtual void disable() = 0;

        virtual uint32_t getPin() const = 0;

        virtual uint32_t getPort() const = 0;

        virtual uint32_t getPortAndPin() const = 0;

        virtual void setMode(PinMode mode) = 0;
        
        virtual void setDriveMode(DriveMode mode) = 0;

        //virtual PullMode getPullMode() = 0;
        virtual void setPullMode(PullMode mode) = 0;

        inline void configureOutput(DriveMode driveMode = DriveMode::PushPull, PullMode pullMode = PullMode::None) {
            setPullMode(pullMode);
            setDriveMode(driveMode);
            setMode(PinMode::Output);
        }

        inline void configureInput(PullMode pullMode = PullMode::None) {
            setPullMode(pullMode);
            setMode(PinMode::Input);
        }

    };
}
