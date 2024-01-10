#pragma once

#include "TimerCh32.h" 
#include <functional>
#include "delayCustom.h"

namespace ES::Driver::Telemetry::Dshot {

    static TIM_TypeDef* timDshot = TIM2;

    static constexpr uint8_t FrameSize = 16;

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
        uint16_t throttle = 0;
        uint16_t crc = 0;
        bool requestTelemetry = false;
    };

    class DshotListner {
    public:

        DshotListner(const std::function<void(uint16_t throttle)>& handler);

        void callbackCc1(uint16_t value) {
            if(value < 180 && value > 0) {
                _periodLength = value;
            }
            if(_periodLength != 0) {
                if(value > 200) {
                    Delay::delayUs(70);
                    TIM_DMACmd(TIM2, TIM_DMA_CC2, ENABLE);
                    TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
                }
            }
        }

        void enable() {
            _inputCap.start();
        }

        void disable() {
            _inputCap.stop();
        }

        void callbackDma() {
            parseBuffer();
        }

        bool parseBuffer() {
            uint16_t value = 0;
            uint16_t valueForCrc = 0;
            uint16_t crc = 0;
            for(uint8_t i = 0; i < ThrottleBits; i++) {
                if(_fallingEdges[i] > (_periodLength / 2)) {
                    value |=  1 << (ThrottleBits - 1) - i;
                    valueForCrc |= 1 << i;
                }
            }
            valueForCrc << 1;
            if(_fallingEdges[TelemetryMask] > (_periodLength / 2)) {
                value |=  1 << TelemetryMask;
                valueForCrc |= 1;
            }
            for(uint8_t i = TelemetryMask + 1; i < FrameSize; i++) {
                if(_fallingEdges[i] > (_periodLength / 2)) {
                    //value |=  1 << (FrameSize - 1) - (i - (TelemetryMask + 1));
                    value |=  1 << i;
                }
            }
            _packet.crc = (value & CrcMask) >> 12;
            crc = calcCrc(valueForCrc);
            if(crc == _packet.crc) {
                _packet.throttle = value & ThrottleMask;
                _packet.requestTelemetry = value & TelemetryMask;
                _packetRecievedHandler(_packet.throttle);
                return true;
            }
            else {
                _periodLength = 0;
                TIM_DMACmd(TIM2, TIM_DMA_CC2, DISABLE);
                TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
            }
            return false;
        }

        uint16_t calcCrc(uint16_t throttleAndTelemetry) {
            uint16_t value = throttleAndTelemetry;
            return (value ^ (value >> 4) ^ (value >> 8)) & 0x0F;
        }

    private:

        uint16_t _fallingEdges[FrameSize* 2];
        uint16_t _risingEdges[FrameSize* 2];
        Timer::TimerInputCapture _inputCap{timDshot, GPIOA, GPIO_Pin_0, TIM_Channel_1, reinterpret_cast<uint32_t>(_fallingEdges)};

        uint16_t ccValue = 0;
        bool _frameStartFound = false;
        uint16_t _capturedFrame[FrameSize * 2];
        std::function<void(uint16_t throttle)> _packetRecievedHandler{};

        DshotPacket _packet{};
        uint16_t _periodLength = 0;
    };
}