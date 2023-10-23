#pragma once

#include "GpioCh32v.h"
#include "CriticalSection.h"
#include "ExtiCh32v.h"
#include "TimerRtos.h"
#include "Semaphore.h"

#include <bitset>

namespace ES::Driver::Telemetry::Dhsot {

    static constexpr uint8_t FrameSize = 16;
    static constexpr uint8_t EdgeChangePerBit = 2;

    static constexpr uint16_t HighEdgePeriodSet = 1000;
    static constexpr uint16_t HighEdgePeriodReset = 500;
    static constexpr uint16_t BitPeriod = 1500;

    static constexpr uint16_t ThrottleMask = 0x7FF;
    static constexpr uint16_t TelemetryMask = 11;
    static constexpr uint16_t CrcMask = 0xF000;

    struct DshotPacket {
    public:
        uint16_t throttle;
        uint8_t crc;
        bool requetTelemetry;
    }

    class DshotListner : Gpio::InterruptCallback {
    public:
        DshotListner(Gpio::Ch32vPin pin, Threading::BinarySemaphore &frameParsedSem) : _pin(pin), _frameParsedSem(frameParsedSem) {
            Gpio::configureInterrupt(&_pin, this, Gpio::InterruptMode::Rising);
        }

        void onGpioInterrupt(Gpio::Ch32vPin* pin) override {
            if(pin->getPin() == _pin.getPin()) {
                if(!_timer.isStarted()) {
                    if(_pin.read()) {
                        _timer.start();
                    }
                }
                else {
                    if(!_pin.read()) {
                        _timer.stop();
                        if(_timer.getTimeUs() < HighEdgePeriodSet) {
                            _unparsedFrame[_index] = false;
                        }
                        else {
                            _unparsedFrame[_index] = true;
                        }
                        _index++;
                        if(_index == FrameSize) {
                            Threading::CriticalSection lock;
                            parseBuffer();
                        }
                    }
                    if(_pin.read()) {
                        _timer.start();
                    }
                }
            }
        }

        void parseBuffer() {
            _packet.throttle = _unparsedFrame.to_ullong() & ThrottleMask;
            _packet.requestTelemetry = _unparsedFrame[TelemetryBit];
            _packet.crc = _unparsedFrame.to_ullong() & CrcMask;
            _frameParsedSem.give();
        }

        uint16_t getTargerThrottle() {
            return _targetThrottle;
        }

    private:

        std::bitset<FrameSize> _unparsedFrame;
        DshotPacket _packet{};
        uint8_t _index = 0;
        Threading::BinarySemaphore _frameParsedSem;
        Threading::StopWatch _timer {};
        Gpio::Ch32vPin _pin;
    };
}