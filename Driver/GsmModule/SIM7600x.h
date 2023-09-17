#pragma once

#include "UarteNrf52.h"
#include "GpioNrf52.h"
#include "TimerNrf52.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "CriticalSection.h"
#include "BinarySemaphore.h"

#include <array>
#include <string>



namespace ES::Driver {

    const char LF = 0X0A;
    const char CR = 0X0D;

    const std::string AtCommandAt = "AT";
    const char AtCommandGpsEnable[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', '=', '1'};
    const char AtCommandGpsOk[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', '=', '1', CR, CR, LF, 'O', 'K'};
    const char AtCommandGpsInfo[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', 'I', 'N', 'F', 'O'};
    const char AtCommandGpsInfoOk[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', 'I', 'N', 'F', 'O', '?'};
    const char AtCommandGpsDisable[] = {'A', 'T', '+',  'C', 'G', 'P', 'S', '=', '0'};
    const char AtStatusRdy[] = {'R', 'D', 'Y'};
    const char AtCpinReady[] = {'+', 'C', 'P',  'I', 'N', ':', ' ', 'R', 'E', 'A', 'D', 'Y'};
    const char AtSmsDone[] = {'S', 'M', 'S',  ' ', 'D', 'O', 'N', 'E'};
    const char AtPbDone[] = {'P', 'B', ' ', 'D', 'O', 'N', 'E'};
    const char AtStatusReady[] = {'R', 'E', 'A', 'D', 'Y'};
    const char AtStatusOk[] = {'O', 'K'};
    const char AtCrLf[] = {CR, LF};


    enum ModuleEnableStatus {
        Disabled,
        Enabled,
        Boot,
        Failed
    };

    enum ModuleMessagingStatus {
        None,
        WaitingStatus,
        WaitingAtCommandRepeat,
        WaitingReadyStatus,
        WaitingForData
    };

    enum CardinalDirections {
        North,
        South,
        East,
        West
    };

    union Date {
        uint8_t day;
        uint8_t month;
        uint16_t year;
    };

    union Time {
        uint8_t hours;
        uint8_t minutes;
        uint16_t secondsMs;
    };

    struct GpsData {
        public:

        GpsData() = default;

        float latitude;
        CardinalDirections northSouth;
        float longitude;
        CardinalDirections eastWest;
        Date date;
        Time time;
        float altitude;
        float speed;
        float course;
    };
    

    class Sim7600x {
    public:

        Sim7600x(Uarte::UarteNrf uart, Timer::TimerNrf52 timer, Gpio::Nrf52Gpio nDisable/*, Gpio::Nrf52Gpio nReset, Gpio::Nrf52Gpio levelConvEn, Gpio::Nrf52Gpio modulePowerEn, Gpio::Nrf52Gpio ldo1V8En*/) : _uart(uart), _nDisable(nDisable), _timer(timer)/*, _nReset(nReset), _levelConvEn(levelConvEn), _modulePowerEn(modulePowerEn), _ldo1V8En(ldo1V8En) */{

        }

        void enableModule() {
            std::fill(_recieveBuf.begin(), _recieveBuf.end(), 0);
            _uart.init(eventHandler, this); 
            _uart.getStream(std::begin(_recieveByte), 1); // crlf first
            _timer.init(10, timerEventHandler, this);
            //_modulePowerEn.set();
            _nDisable.set();
            _enableStatus = ModuleEnableStatus::Disabled;
            while(_enableStatus != ModuleEnableStatus::Enabled) {
                vTaskDelay(100);
            }
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
 
        ret_code_t sendString(const char * s, size_t size) {
            ret_code_t status = true;
            //return _uart.writeStream(_buffer.begin(), ++index);
            status &= sendCrLf();
            status &=_uart.writeStream(s, size);
            status &= sendCrLf();
            return status;
        }

        ret_code_t sendCrLf() {
            return _uart.writeStream(AtCrLf, 2);
        }

        void handleEventUart(nrfx_uarte_event_t const * p_event) { 
            if(p_event->type == NRFX_UARTE_EVT_RX_DONE) {
                if(*p_event->data.rxtx.p_data == 0) {
                    asm("nop");
                    _uart.getStream(std::begin(_recieveByte), 1);
                }
                else {
                    _recieveBuf[_bufIndex] = _recieveByte[0];
                    _bufferEmpty = false;
                    nextRecieve();
               }
            }
        }

        void handleEventTImer(nrf_timer_event_t p_event) { 
            if(p_event == NRF_TIMER_EVENT_COMPARE0) {
                _onRecieve = false;
                _timer.stop();
                messageRecSem.give();
            }
        }

        void stateMachine() {
            {  
                Threading::CriticalSection lock;
                std::fill(_parseBuf.begin(), _parseBuf.end(), 0);
                std::copy(std::begin(_recieveBuf), std::begin(_recieveBuf) + _bufIndex, std::begin(_parseBuf));
                std::fill(_recieveBuf.begin(), std::begin(_recieveBuf) + _bufIndex, 0);
                _bufIndex = 0;
            }
            _bufferEmpty = true;
            size_t bootDone = false;
            if(_enableStatus == ModuleEnableStatus::Disabled) {
                if(checkAT(AtStatusRdy, false)) {
                    bootDone++;
                    _enableStatus = ModuleEnableStatus::Boot;
                }
                if(_cPinReady == false) {
                    _cPinReady = checkAT(AtCpinReady, false);  
                    bootDone++;
                }
                if(_smsReady == false) {
                    _smsReady = checkAT(AtSmsDone, false);  
                    bootDone++;
                }
                if(_pbReady == false) {
                    _pbReady = checkAT(AtPbDone, true);  
                    bootDone++;
                }
                std::fill(_parseBuf.begin(), _parseBuf.end(), 0);
                if(bootDone == 4) {
                    _enableStatus = ModuleEnableStatus::Enabled;
                }
            }
            if(_messagingStatus == ModuleMessagingStatus::WaitingReadyStatus) {
                if(checkAT(_atCommandForCheck, true)) {
                    _messagingStatus = ModuleMessagingStatus::None;
                    _readyStatusRecieved.give();
                }
            }
            if(_messagingStatus == ModuleMessagingStatus::WaitingAtCommandRepeat) {
                if(checkAT(_atCommandForCheck, false)) {
                    _messagingStatus = ModuleMessagingStatus::WaitingForData;
                }
            }
            if(_messagingStatus == ModuleMessagingStatus::WaitingForData) {
                parseData();
                _messagingStatus = ModuleMessagingStatus::None;
                _dataRecieved.give();
            }
        }

        void nextRecieve() {
            if(!_onRecieve) {
                _onRecieve = true;
                _timer.start();
            }
            else{
                _timer.clear();
            }
            _bufIndex++;
            _uart.getStream(std::begin(_recieveByte), 1);
        }

        bool checkAT(const char* s, bool resetIndex) {
            bool status = true;
            if(_parseBuf[_parserIndex] == CR) {
                _parserIndex++;
            }
            if(_parseBuf[_parserIndex] == LF) {
                _parserIndex++;
            }
            for(uint8_t i = 0; _parseBuf[_parserIndex] != CR; _parserIndex++, i++) {
                if(s[i] == '\0') {
                    return false;
                }
                if(s[i] != static_cast<char>(_parseBuf[_parserIndex])) {
                    return false;
                }
            }
            _parserIndex++;
            if(_parseBuf[_parserIndex] == LF) {
                _parserIndex++;
            }
            if(_parseBuf[_parserIndex] == 0) {
                _bufferEmpty = true;
            }
            if(resetIndex) {
                _parserIndex = 0;
            }
            return status;
        }

        bool enableGps() {
            ret_code_t status;
            if(_messagingStatus == ModuleMessagingStatus::None && !_gpsReady) {
                status = sendString(AtCommandGpsEnable, sizeof(AtCommandGpsEnable));
                _atCommandForCheck = AtCommandGpsOk;
                _messagingStatus = ModuleMessagingStatus::WaitingReadyStatus;
                size_t counter = 20;
                while(counter--) {
                    if(_readyStatusRecieved.take(100)) {
                        _gpsReady = true;
                        break;
                    }
                }
            }
            _atCommandForCheck = nullptr;
            return _gpsReady;
        }

        bool getLocation() {
            if(!_gpsReady) {
                return false;
            }
            ret_code_t status;
            if(_messagingStatus == ModuleMessagingStatus::None) {
                status = sendString(AtCommandGpsInfo, sizeof(AtCommandGpsInfo));
                _atCommandForCheck = AtCommandGpsInfo;
                _messagingStatus = ModuleMessagingStatus::WaitingAtCommandRepeat;
            }
            size_t counter = 20;
            while(counter--) {
                if(_dataRecieved.take(100)) {
                    break;
                }
            }
        }

        bool parseData() {

        }

        Threading::BinarySemaphore messageRecSem;
        
    private:
    
        static void eventHandler(nrfx_uarte_event_t const * p_event, void * p_context) {
            auto _this = reinterpret_cast<Sim7600x*>(p_context);
            _this->handleEventUart(p_event);
        }

        static void timerEventHandler(nrf_timer_event_t event_type, void* p_context){
            auto _this = reinterpret_cast<Sim7600x*>(p_context);
            _this->handleEventTImer(event_type);
		}

    private:

        GpsData _gpsData;

        Threading::BinarySemaphore _readyStatusRecieved;
        Threading::BinarySemaphore _dataRecieved;

        Uarte::UarteNrf _uart;
        Gpio::Nrf52Gpio _nDisable;
        //Gpio::Nrf52Gpio _nReset;
        //Gpio::Nrf52Gpio _levelConvEn;
        //Gpio::Nrf52Gpio _modulePowerEn;
       // Gpio::Nrf52Gpio _ldo1V8En;


        Timer::TimerNrf52 _timer;
        size_t _bufIndex = 0;
        uint8_t _parserIndex = 0;
        std::array<uint8_t, 1> _recieveByte;
        std::array<uint8_t, 128> _recieveBuf;
        std::array<uint8_t, 128> _parseBuf;
        std::array<char, 128> _buffer;
        const char * _atCommandForCheck = nullptr;

        bool _bufferEmpty = true;
        bool _onRecieve = false;

        bool _cPinReady = false;
        bool _smsReady = false;
        bool _pbReady = false;
        bool _gpsReady = false;
        
        bool _readyStatus = false;

        ModuleEnableStatus _enableStatus = ModuleEnableStatus::Disabled;
        ModuleMessagingStatus _messagingStatus = ModuleMessagingStatus::None;
    };

}