#pragma once

#include "ThreadFreeRtos.h"
#include "FreeRtosQueue.h"

namespace ES::Driver::Telemetry::Dhsot {

    static TIM_TypeDef* timMeas = TIM2;

    static constexpr uint8_t FrameSize = 16;

    static constexpr uint16_t HighEdgePeriodSet = 1000;
    static constexpr uint16_t HighEdgePeriodReset = 500;
    static constexpr uint16_t BitPeriod = 1500;

    static constexpr uint16_t ThrottleMask = 0x7FF;
    static constexpr uint16_t TelemetryMask = 11;
    static constexpr uint16_t CrcMask = 0xF000;

    static constexpr uint8_t ThrottleBits = 11;

    enum CommandType {
        Command,
        Throttle
    };

    struct DshotPacket {
    public:
        uint16_t throttle;
        uint16_t crc;
        bool requestTelemetry;
    };

    class DshotListner {
    public:

        DshotListner();

        void callbackCc1(uint16_t value) {
            if(_frameStartFound) {
                _capturedFrame[_index] = value;
                _index++;
                if(_index >= FrameSize) {
                    _index = 0;
                    flag = true;
                    _frameStartFound = false;
                }
            }
        }

        void callbackCc2(uint16_t value) {
            if(value > 100) {
                _frameStartFound = true;
                _index = 0;
            }
            else if(value != 0){
                _periodLength = value;
            }
        }

        bool parseBuffer() {
            uint16_t value = 0;
            uint16_t valueForCrc = 0;
            uint16_t crc = 0;
            for(uint8_t i = 0; i < ThrottleBits; i++) {
                if(_capturedFrame[i] > (_periodLength / 2)) {
                    value |=  1 << (ThrottleBits - 1) - i;
                    valueForCrc |= 1 << i;
                }
            }
            valueForCrc << 1;
            if(_capturedFrame[TelemetryMask] > (_periodLength / 2)) {
                value |=  1 << TelemetryMask;
                valueForCrc |= 1;
            }
            for(uint8_t i = TelemetryMask + 1; i < FrameSize; i++) {
                if(_capturedFrame[i] > (_periodLength / 2)) {
                    //value |=  1 << (FrameSize - 1) - (i - (TelemetryMask + 1));
                    value |=  1 << i;
                }
            }
            _packet.crc = (value & CrcMask) >> 12;
            crc = calcCrc(valueForCrc);
            if(crc == _packet.crc) {
                _packet.throttle = value & ThrottleMask;
                _packet.requestTelemetry = value & TelemetryMask;
                return true;
            }
            return false;
        }

        uint16_t calcCrc(uint16_t throttleAndTelemetry) {
            uint16_t value = throttleAndTelemetry;
            return (value ^ (value >> 4) ^ (value >> 8)) & 0x0F;
        }

        CommandType getThrottle(uint16_t& value) {
            value = _packet.throttle;
            if(value == 0) {
                return CommandType::Throttle;
            }
            else if(_packet.throttle < 48) {
                return CommandType::Command;
            } 
            else {
                return CommandType::Throttle;
            }

        }

    private:

        Threading::Thread _DhshotHandle{"Dhsot", 256, Threading::ThreadPriority::Normal,  [this](){
            while(true) {
                if(flag) {
                    parseBuffer();
                    flag = false;
                }
                Threading::yield();
            }
        }};

        uint16_t ccValue = 0;
        bool flag = false;
        bool _frameStartFound = false;
        uint16_t _capturedFrame[FrameSize * 2];
        DshotPacket _packet{};
        uint8_t _index = 0;
        uint16_t _periodLength = 0;
    };
}