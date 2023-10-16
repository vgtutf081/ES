#pragma once

namespace ES::Driver::Timer {
    class ITimer {
    public:

    };

    class IPwm {
    public:
        virtual void start() = 0;
        virtual void stop() = 0;
        //virtual bool setParams(uint32_t freqHz, float duty) = 0;
    };


}