#pragma once

#include "UarteNrf52.h"
#include "GpioNrf52.h"
#include "TimerNrf52.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "CriticalSection.h"
#include "Semaphore.h"

#include <array>
#include <string>
#include <cmath>

#include "nrf_check_ret.h"

#include "CommonTools.h"

namespace ES::Driver {

    const char LF = 0X0A;
    const char CR = 0X0D;
    
    const std::string AtCommandAt = "AT";
    const char Tele2Operator[] = {'"', 'T', 'e', 'l', 'e', '2', ' ', 'T', 'e', 'l', 'e', '2', '"'};
    const char AtCopsData[] = {'+', 'C', 'O', 'P', 'S', ':', ' '}; 
    const char AtCopsRead[] = "AT+COPS?";
    const char TestNumber[] = {'+','7','9','0','8','1','4','6','0','3','5','6'};
    const char ATD[] = {'A', 'T', 'D'}; //make call
    const char ATA[] = {'A', 'T', 'A'}; //answer call
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
    const char AtStatusOk[] = "OK";
    const char AtCrLf[] = {CR, LF};

    const char AtdTest[] = "ATD+79081460356;";
    const char VoiceCallBegin[] = "VOICE CALL: BEGIN";
    const char VoiceCallEnd[] = "VOICE CALL: END";
    const char NoCarrier[] = "NO CARRIER";

    enum ModuleEnableStatus {
        Disabled,
        Enabled,
        Boot,
        Failed
    };

    enum ModuleStatus {
        None,
        WaitingStatus,
        WaitingAtCommandRepeat,
        WaitingReadyStatus,
        WaitingForOk,
        WaitingForData,
        Idle,
        PreCall,
        OutgoingCall,
        IncomingCall
    };

    enum DataType {
        GpsData,
        CopsData,
        NoneData,
        CallData
    };

    enum CardinalDirections {
        North,
        South,
        East,
        West
    };

    enum ATS : uint8_t {
        Gsm  = 0,
        GsmCOmpact = 1,
        Utran = 2,
        Eutran = 7,
        Cdma_Hdr = 8
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

        uint8_t counterTest = 0;

        Sim7600x(Uarte::UarteNrf uart, Timer::TimerNrf52 timer, Gpio::Nrf52Gpio nDisable/*, Gpio::Nrf52Gpio nReset, Gpio::Nrf52Gpio levelConvEn, Gpio::Nrf52Gpio modulePowerEn, Gpio::Nrf52Gpio ldo1V8En*/) : _uart(uart), _nDisable(nDisable), _timer(timer)/*, _nReset(nReset), _levelConvEn(levelConvEn), _modulePowerEn(modulePowerEn), _ldo1V8En(ldo1V8En) */{

        }

        void enableModule() {
            std::fill(_recieveBuf.begin(), _recieveBuf.end(), 0);
            _uart.init(eventHandler, this); 
            _uart.getStream(std::begin(_recieveByte), 1); // crlf first
            _timer.init(1, timerEventHandler, this);
            //_modulePowerEn.set();
            _nDisable.configureOutput();
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

        bool sendCommand(const char * s) {
            bool status = sendString(s, 2);
            _atCommandForCheck = s;
            _moduleStatus = ModuleStatus::WaitingAtCommandRepeat;
            size_t counter = 20;
            while(counter--) {
                if(_commandConfirmed.take(100)) {
                    _atCommandForCheck = nullptr;
                    _moduleStatus = ModuleStatus::Idle;
                    return true;
                }
            }
            return false;
        }

        bool checkCallsAvailability() {
            bool status = false;
            if(_enableStatus == ModuleEnableStatus::Enabled) {
                if(!sendCommand(AtCopsRead)) {
                    return false;
                }
                _moduleStatus = ModuleStatus::WaitingForData;
                _dataType = DataType::CopsData;
                size_t counter = 20;
                while(counter--) {
                    if(_dataRecieved.take(100)) {
                        status = true;
                    }  
                }
            }
            _moduleStatus = ModuleStatus::Idle;
            _dataType = DataType::NoneData;
            return status;
        }
 
        ret_code_t sendString(const char * s, size_t size) {
            ret_code_t status;
            size_t size2 = CommonTools::charArraySize(s);
            char tempBuf[128];
            memcpy(tempBuf, AtCrLf, 2);
            memcpy(&tempBuf[2], s, size2);
            memcpy(&tempBuf[size2 + 2], AtCrLf, 2);
            //return _uart.writeStream(_buffer.begin(), ++index);
            //tatus = sendCrLf();
            status =_uart.writeStream(tempBuf, size2 + 4);
            //status = sendCrLf();
            return CheckErrorCode::success(status);
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
            std::fill(_parseBuf.begin(), _parseBuf.end(), 0);
            {  
                Threading::CriticalSection lock;
                std::copy(std::begin(_recieveBuf), std::begin(_recieveBuf) + _bufIndex, std::begin(_parseBuf));
                std::fill(_recieveBuf.begin(), std::begin(_recieveBuf) + _bufIndex, 0);
                _bufIndex = 0;
            }
            _bufferEmpty = true;
            size_t bootDone = false;
            if(_enableStatus == ModuleEnableStatus::Disabled) {
                if(checkAT(AtStatusRdy)) {
                    bootDone++;
                    _enableStatus = ModuleEnableStatus::Boot;
                }
                if(_cPinReady == false) {
                    _cPinReady = checkAT(AtCpinReady);  
                    bootDone++;
                }
                if(_smsReady == false) {
                    _smsReady = checkAT(AtSmsDone);  
                    bootDone++;
                }
                if(_pbReady == false) {
                    _pbReady = checkAT(AtPbDone);  
                    bootDone++;
                }
                if(bootDone == 4) {
                    _enableStatus = ModuleEnableStatus::Enabled;
                }
                return;
            }
            if(_moduleStatus == ModuleStatus::WaitingReadyStatus) {
                if(checkAT(_atCommandForCheck)) {
                    _moduleStatus = ModuleStatus::None;
                    _readyStatusRecieved.give();
                }
                return;
            }
            if(_moduleStatus == ModuleStatus::WaitingForOk) {
                if(checkAT(AtStatusOk)) {
                    _okRecieved.give();
                }
                return;
            }
            if(_moduleStatus == ModuleStatus::WaitingAtCommandRepeat) {
                if(checkAT(_atCommandForCheck)) {
                    _commandConfirmed.give();
                }
                return;
            }
            if(_moduleStatus == ModuleStatus::WaitingForData) {
                if(parseData()) {
                    _dataRecieved.give();
                }
                _moduleStatus = ModuleStatus::Idle;
                return;
            }
            if(_moduleStatus == ModuleStatus::Idle) {
                if(checkAT(_atCommandForCheck)) {
                    //_moduleStatus = ModuleStatus::Ring;
                    incomingCall.give();
                }
                return;
            }
            if(_moduleStatus == ModuleStatus::PreCall) {
                if(checkAT(VoiceCallBegin)) {
                    _moduleStatus = ModuleStatus::OutgoingCall;
                }
                if(checkAT(NoCarrier)) {
                    _moduleStatus = ModuleStatus::Idle;
                }
                return;
            }
            if(_moduleStatus == ModuleStatus::OutgoingCall) {
                if(checkAT(VoiceCallEnd)) {
                    _dataType = DataType::CallData;
                    _moduleStatus = ModuleStatus::Idle;
                    parseData();
                    _dataType = DataType::NoneData;
                }
                return;
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

        bool checkAT(const char* s) {
            bool status = true;
            skipCrLf();
            /*if(_parseBuf[_parserIndex] == CR) {
                _parserIndex++;
            }
            if(_parseBuf[_parserIndex] == CR) {
                _parserIndex++;
            }
            if(_parseBuf[_parserIndex] == LF) {
                _parserIndex++;
            }*/
            for(uint8_t i = 0; _parseBuf[_parserIndex] != CR; _parserIndex++, i++) {
                if(s[i] == '\0') {
                    break;
                }
                if(s[i] != static_cast<char>(_parseBuf[_parserIndex])) {
                    return false;
                }
            }
            _parserIndex++;
            skipCrLf();
            /*if(_parseBuf[_parserIndex] == CR) {
                _parserIndex++;
            }
            if(_parseBuf[_parserIndex] == CR) {
                _parserIndex++;
            }
            if(_parseBuf[_parserIndex] == LF) {
                _parserIndex++;
            }*/
            if(_parseBuf[_parserIndex] == 0) {
                _bufferEmpty = true;
                _parserIndex = 0;
            }
            return status;
        }

        void skipCrLf() {
            while(_parseBuf[_parserIndex] == CR || _parseBuf[_parserIndex] == LF) {
                _parserIndex++;
            }
        }

        bool enableGps() {
            ret_code_t status;
            if(_moduleStatus == ModuleStatus::None && !_gpsReady) {
                status = sendString(AtCommandGpsEnable, sizeof(AtCommandGpsEnable));
                _atCommandForCheck = AtCommandGpsOk;
                _moduleStatus = ModuleStatus::WaitingReadyStatus;
                size_t counter = 20;
                while(counter--) {
                    if(_readyStatusRecieved.take(100)) {
                        _gpsReady = true;
                        break;
                    }
                }
            }
            _atCommandForCheck = nullptr;
            return CheckErrorCode::success(status);
        }

        bool getLocation() {
            if(!_gpsReady) {
                return false;
            }
            ret_code_t status;
            if(_moduleStatus == ModuleStatus::None) {
                status = sendString(AtCommandGpsInfo, sizeof(AtCommandGpsInfo));
                _atCommandForCheck = AtCommandGpsInfo;
                _moduleStatus = ModuleStatus::WaitingAtCommandRepeat;
            }
            size_t counter = 20;
            while(counter--) {
                if(_dataRecieved.take(100)) {
                    break;
                }
            }
            return CheckErrorCode::success(status);
        }

        bool parseData() {
            if(_dataType == DataType::CopsData) {
                bool status = false;
                if(checkAT(AtCopsData)) {
                    _parserIndex++;
                    _gsmOperatorSelectionMode = _parseBuf[_parserIndex] - '0';
                    _parserIndex += 2;
                    if(checkAT(Tele2Operator)) {
                        _callsAvailable = true;
                    }
                    else {
                        _callsAvailable = false;
                    }
                    _selectedTechnology = static_cast<ATS>(_parseBuf[_parserIndex] - '0');
                    _parserIndex += 3;

                    if(checkAT(AtStatusOk)) {
                        status = true;
                    }
                    _parserIndex = 0;
                    _dataType = DataType::NoneData;
                }
            }
            if(_dataType == DataType::CallData) {
                _parserIndex++;
                _callLengthSeconds = 0;
                for(int i = 5; i > -1; i--) {
                    _callLengthSeconds += (_parseBuf[_parserIndex + i] - '0') * pow(10, 5 - i);
                }
            }
            return true;
        }

        bool makeCall() {
            auto status = sendCommand(AtdTest);
            if(!status) {
                return false;
            }
            size_t count = 20;
            _moduleStatus = ModuleStatus::WaitingForOk;
            while(count--) {
                if(_okRecieved.take(100)) {
                    break;
                }
            }
            if(count == 0) {
                return false;
            }
            _moduleStatus = ModuleStatus::PreCall;
            return status;
        }

        bool answerCall() {
            bool status = sendString(ATA, sizeof(ATA));
            return CheckErrorCode::success(status);
        }

        ModuleStatus getStatus() {
            return _moduleStatus;
        }

        ModuleEnableStatus getEnableStatus() {
            return _enableStatus;
        }

        bool getCallsAvailable() {
            return _callsAvailable;
        }

        Threading::BinarySemaphore messageRecSem;
        Threading::BinarySemaphore incomingCall;
        
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

        DataType _gpsData;

        Threading::BinarySemaphore _readyStatusRecieved;
        Threading::BinarySemaphore _commandConfirmed;
        Threading::BinarySemaphore _dataRecieved;
        Threading::BinarySemaphore _okRecieved;

        Uarte::UarteNrf _uart;
        Gpio::Nrf52Gpio _nDisable;
        //Gpio::Nrf52Gpio _nReset;
        //Gpio::Nrf52Gpio _levelConvEn;
        //Gpio::Nrf52Gpio _modulePowerEn;
       // Gpio::Nrf52Gpio _ldo1V8En;

        uint8_t _gsmOperatorSelectionMode;
        bool _callsAvailable = false;
        ATS _selectedTechnology;

        uint64_t _callLengthSeconds = 0;

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
        ModuleStatus _moduleStatus = ModuleStatus::None;
        DataType _dataType = DataType::NoneData;
    };

}