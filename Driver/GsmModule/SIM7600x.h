#pragma once

#include "UarteNrf52.h"
#include "GpioNrf52.h"
#include "task.h"

#include <array>
#include <string>

#include "FreeRTOS.h"

namespace ES::Driver {

    static const std::string crlf = "\r\n";

    const std::string AtCommand = "AT" + crlf;
    const std::string AtGpsEnable = "AT+CGPS=1" + crlf;
    

    class Sim7600x {
    public:
        Sim7600x(Uarte::UarteNrf uart/*, Gpio::Nrf52Gpio nDisable, Gpio::Nrf52Gpio nReset, Gpio::Nrf52Gpio levelConvEn, Gpio::Nrf52Gpio modulePowerEn, Gpio::Nrf52Gpio ldo1V8En*/) : _uart(uart)/*, _nDisable(nDisable), _nReset(nReset), _levelConvEn(levelConvEn), _modulePowerEn(modulePowerEn), _ldo1V8En(ldo1V8En) */{
            //_ldo1V8En.set();
            vTaskDelay(10);
            //_levelConvEn.set();
            vTaskDelay(10);
        }

        void enableModule() {
            //_modulePowerEn.set();
            vTaskDelay(50);
            //_nDisable.set();
            vTaskDelay(10);
            _uart.getStream(std::begin(_recieveBuf), 1);
        }

        void disableModule() {
            //_nDisable.reset();
            vTaskDelay(50);
            //_modulePowerEn.reset();
        }

        void resetModule() {
            //_nReset.set();
            vTaskDelay(100);
            //_nReset.reset();
        }

        void sendString(const std::string& s) {
            //std::fill(_sendBuffer.begin(), _sendBuffer.end(), 0);
            strcpy(_sendBuffer.begin(), s.c_str());
            _uart.writeStream(_sendBuffer.begin(), s.size());
        }

        void handleEvent(nrfx_uarte_event_t const * p_event) {

        }

        void nextRecieve() {
            _bufIndex++;
            _uart.getStream(std::begin(_recieveBuf) + _bufIndex, 1);
        }

    private:
        Uarte::UarteNrf _uart;
       // Gpio::Nrf52Gpio _nDisable;
        //Gpio::Nrf52Gpio _nReset;
        //Gpio::Nrf52Gpio _levelConvEn;
        //Gpio::Nrf52Gpio _modulePowerEn;
       // Gpio::Nrf52Gpio _ldo1V8En;

        
        size_t _bufIndex = 0;
        std::array<uint8_t, 256> _recieveBuf;
        std::array<char, 256> _sendBuffer;

    };
}