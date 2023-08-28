#pragma once

#include "UarteNrf52.h"
#include "GpioNrf52.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <array>
#include <string>



namespace ES::Driver {

    const std::string AtCommandAt = "AT";
    const std::string AtCommandGpsEnable = "AT+CGPS=1";
    const std::string AtStatusRdy = "RDY";
    const char LF = 0X0A;
    const char CR = 0X0D;

    enum ModuleEnableStatus {
        Disabled,
        Enabled,
        Enabling,
        Failed
    };

    enum ModuleMessagingStatus {
        Boot,
        WaitingStatus,
        WaitingAtCommandRepeat,
        WaitingReadyStatus
    };
    

    class Sim7600x {
    public:

        Sim7600x(Uarte::UarteNrf uart, Gpio::Nrf52Gpio nDisable/*, Gpio::Nrf52Gpio nReset, Gpio::Nrf52Gpio levelConvEn, Gpio::Nrf52Gpio modulePowerEn, Gpio::Nrf52Gpio ldo1V8En*/) : _uart(uart), _nDisable(nDisable)/*, _nReset(nReset), _levelConvEn(levelConvEn), _modulePowerEn(modulePowerEn), _ldo1V8En(ldo1V8En) */{
            messageRecieveSem = xSemaphoreCreateBinary();

        }

        void enableModule() {
            _uart.init(eventHandler, this); 
            _uart.getStream(std::begin(_recieveBuf), 1); // crlf first
            //_modulePowerEn.set();
            _nDisable.set();
            _enableStatus = ModuleEnableStatus::Enabling;
            vTaskDelay(100);
        }

        void disableModule() {
            _nDisable.reset(); //TODO invert
            vTaskDelay(50);
            //_modulePowerEn.reset();
        }

        void resetModule() {
            //_nReset.set();
            vTaskDelay(100);
            //_nReset.reset();
        }

        ret_code_t sendString(const std::string& s) {
            std::fill(_sendBuffer.begin(), _sendBuffer.end(), 0);
            strcpy(_sendBuffer.begin(), s.c_str());
            size_t index = s.size();
            _sendBuffer[index] = CR;
            _sendBuffer[++index] = LF;
            return _uart.writeStream(_sendBuffer.begin(), ++index);
        }

        void handleEvent(nrfx_uarte_event_t const * p_event) { 
            if(p_event->type == NRFX_UARTE_EVT_RX_DONE) {
                nextRecieve();
            if(counter == 0) {
                xSemaphoreGive(messageRecieveSem);
            }
                /*else if(_crLfRequired) {
                    //_crLfRequired = false;
                    if(_messagingStatus == ModuleMessagingStatus::Boot) {
                        _messagingStatus = ModuleMessagingStatus::WaitingReadyStatus;
                        _uart.getStream(std::begin(_recieveBuf), 3);
                    }
                }*/
                /*else{
                    xSemaphoreGive(messageRecieveSem);
                    _bufIndex = 0;
                }*/
            }
        }

        void stateMachine() {
            if(_enableStatus == ModuleEnableStatus::Enabling) {
                if(_messagingStatus == ModuleMessagingStatus::WaitingReadyStatus) {
                    if(getAtCommandFromRecieveBuf() == "RDY") {
                        _enableStatus = ModuleEnableStatus::Enabled;
                    }
                    else {
                        asm("nop");
                    }
                }
            }
        }

        void nextRecieve() {
            _bufIndex++;
            _uart.getStream(std::begin(_recieveBuf) + _bufIndex, 1);

            counter--;
        }

        std::string getAtCommandFromRecieveBuf() {
            std::string s;
            for(size_t i = 0; _recieveBuf[i] != CR; i++) {
                s += static_cast<char>(_recieveBuf[i]);
            }
            std::fill(_recieveBuf.begin(), _recieveBuf.end(), 0);
            return s;
        }

        SemaphoreHandle_t messageRecieveSem;
        
    private:
        static void eventHandler(nrfx_uarte_event_t const * p_event, void * p_context) {
            auto _this = reinterpret_cast<Sim7600x*>(p_context);
            _this->handleEvent(p_event);
        }

    private:
        size_t counter = 40;

        Uarte::UarteNrf _uart;
        Gpio::Nrf52Gpio _nDisable;
        //Gpio::Nrf52Gpio _nReset;
        //Gpio::Nrf52Gpio _levelConvEn;
        //Gpio::Nrf52Gpio _modulePowerEn;
       // Gpio::Nrf52Gpio _ldo1V8En;



        size_t _bufIndex = 0;
        std::array<uint8_t, 256> _recieveBuf;
        std::array<char, 256> _sendBuffer;

        ModuleEnableStatus _enableStatus = ModuleEnableStatus::Disabled;
        ModuleMessagingStatus _messagingStatus  = ModuleMessagingStatus::Boot;
        bool _crLfRequired = false;
    };

}