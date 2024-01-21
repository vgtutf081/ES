#pragma once

#include "UarteNrf52.h"
#include "GpioNrf52.h"
#include "TimerNrf52.h"

//#include "FreeRTOS.h"
#include "CriticalSection.h"
#include "Semaphore.h"
#include "ThreadFreeRtos.h"

#include <array>
#include <string>
#include <cmath>

#include "nrf_check_ret.h"

#include "CommonTools.h"

#include "SIM7600AtCommands.h"
#include "SIMTypes.h"

namespace ES::Driver {
  
    static constexpr size_t timeSize = 7;
    static constexpr size_t numberSize = 12;

    class Sim7600x {
    public:

        uint8_t counterTest = 0;
    #if defined(HARDWARE_VERSION_1_0_0)
        Sim7600x(Uarte::UarteNrf uart, Timer::TimerNrf52 timer,
                    Gpio::IGpio& nDisable, Gpio::IGpio& nReset
                    ) : 
                    _uart(uart), _timer(timer),
                    _nDisable(nDisable), _nReset(nReset){
            std::fill(_recieveBuf.begin(), _recieveBuf.end(), 0);
            _uart.init(eventHandler, this); 
            _uart.getStream(std::begin(_recieveByte), 1); // crlf first
            _timer.init(1, timerEventHandler, this);
        }
    #else
        Sim7600x(Uarte::UarteNrf uart, Timer::TimerNrf52 timer, Gpio::IGpio& nDisable/*, Gpio::Nrf52Gpio nReset, Gpio::Nrf52Gpio levelConvEn, Gpio::Nrf52Gpio modulePowerEn, Gpio::Nrf52Gpio ldo1V8En*/) : _uart(uart), _nDisable(nDisable), _timer(timer)/*, _nReset(nReset), _levelConvEn(levelConvEn), _modulePowerEn(modulePowerEn), _ldo1V8En(ldo1V8En) */{

        }
    #endif

        void enableModule() {
            //_modulePowerEn.set();

            //doesn`t work in my PCB
            // _nReset.configureOutput();
            // _nReset.set();
            // _nReset.reset();

            // _nDisable.configureOutput();
            // _nDisable.set();

            _enableStatus = ModuleEnableStatus::Disabled;
            while(_enableStatus != ModuleEnableStatus::Enabled) {
                Threading::sleepForMs(100);
            }
        }

        void disableModule() {
            _nDisable.reset(); //TODO invert
            Threading::sleepForMs(50);
            //_modulePowerEn.reset();
        }

        void resetModule() {
            //_nReset.set();
            Threading::sleepForMs(100);
            //_nReset.reset();
        }

        bool sendCommand(const char * s, const char * expectedAnswer = nullptr) {
            bool status = sendString(s, 2);
            _atCommandForCheck = s;
            _moduleStatus = ModuleStatus::WaitingAtCommandRepeat;
            status &= _commandConfirmed.take(1000);
            _atCommandForCheck = nullptr;
            if(expectedAnswer != nullptr) {
                status &= checkAT(expectedAnswer);
            }
            _moduleStatus = ModuleStatus::None;
            return status;
        }

        bool checkCallsAvailability() {
            bool status = false;
            if(_enableStatus == ModuleEnableStatus::Enabled) {
                if(!sendCommand(AtCopsRead)) {
                    return false;
                }
                _moduleStatus = ModuleStatus::WaitingForData;
                size_t counter = 20;
                while(counter--) {
                    if(_dataRecieved.take(100)) {
                        status = parseData(DataType::CopsData);
                        _moduleStatus = ModuleStatus::None;
                        break;
                    }  
                }
            }
            return status;
        }
 
        ret_code_t sendString(const char * s, size_t size) {
            ret_code_t status;
            size_t size2 = CommonTools::charArraySize(s);
            char tempBuf[128];
            memcpy(tempBuf, AtCrLf, 2);
            memcpy(&tempBuf[2], s, size2);
            memcpy(&tempBuf[size2 + 2], AtCrLf, 2);
            status =_uart.writeStream(tempBuf, size2 + 4);
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
                    nextRecieve();
               }
            }else if(p_event->type == NRFX_UARTE_EVT_TX_DONE){

            }else{
                //error state
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
                _parserIndex = 0;
            }
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
                if(checkForOk()) {
                    _okRecieved.give();
                }
                return;
            }
            if(_moduleStatus == ModuleStatus::WaitingAtCommandRepeat) {
                if(checkAT(_atCommandForCheck)) {
                    _commandConfirmed.give();
                }
                /*if(_parserIndex != 0) {
                    if(checkAT(AtStatusOk)) {
                        _okRecieved.give();
                    }
                }*/
                return;
            }
            if(_moduleStatus == ModuleStatus::WaitingForData) {
                _dataRecieved.give();
                _moduleStatus = ModuleStatus::None;
                return;
            }
            if(_moduleStatus == ModuleStatus::None) {
                if(_phoneStatus == PhoneStatus::Idle) {
                    if(checkAT(Ring)) {
                        _phoneStatus = PhoneStatus::IncomingPreCall;
                        return;
                    }
                }
            }
            if(_phoneStatus == PhoneStatus::IncomingPreCall) {
                if(checkAT(VoiceCallBegin)) {
                    _phoneStatus = PhoneStatus::IncomingCall;
                    return;
                }
                if(checkAT(MissedCall)) {
                    _phoneStatus = PhoneStatus::Idle;
                    parseData(DataType::MissedCallData);
                    return;
                }
            }
            if(_phoneStatus == PhoneStatus::OutgoingPreCall) {
                if(checkAT(VoiceCallBegin)) {
                    _phoneStatus = PhoneStatus::OutgoingCall;
                    return;
                }
                if(checkAT(NoCarrier)) {
                    _phoneStatus = PhoneStatus::Idle;
                    return;
                }
            }
            if(_phoneStatus == PhoneStatus::OutgoingCall || _phoneStatus == PhoneStatus::IncomingCall) {
                if(checkAT(VoiceCallEnd)) {
                    _phoneStatus = PhoneStatus::Idle;
                    parseData(DataType::CallData);
                    return;
                }
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
                    parseData(DataType::GpsData);

                    break;
                }
            }
            return CheckErrorCode::success(status);
        }

        bool parseData(DataType dataType) {
            if(dataType == DataType::CopsData) {
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
                }
            }
            if(dataType == DataType::CallData) {
                _parserIndex++;
                _callLengthSeconds = 0;
                for(int i = 5; i > -1; i--) {
                    _callLengthSeconds += (_parseBuf[_parserIndex + i] - '0') * pow(10, 5 - i);
                }
            }
            if(dataType == DataType::MissedCallData) {
                size_t index = 0;
                while(index < timeSize) {
                    _parserIndex++;
                    _missedCallTime[index] = _parseBuf[_parserIndex];
                    index++;
                }
                _parserIndex++;
                index = 0;
                while(index < numberSize) {
                    _parserIndex++;
                    _missedCallNumber[index] = _parseBuf[_parserIndex];
                    index++;
                }
                _parserIndex = 0;
            }
            return true;
        }

        bool makeCall() {
            auto status = sendCommand(AtdTest);
            if(!status) {
                return false;
            }
            status &= checkForOk();
            _phoneStatus = PhoneStatus::OutgoingPreCall;
            return status;
        }

        bool disconnectCall() {
            auto status = sendCommand(SetAthAvilable);
            size_t count = 20;
            while(count--) {
                if(_okRecieved.take(10)) {
                    break;
                }
            }
            if(count == 0) {
                return false;
            }
            status = sendCommand(DisconnectCall);
            _phoneStatus = PhoneStatus::Idle;
            return status;
        }

        bool answerCall() {
            bool status = sendCommand(AnswerCall);
            return status;
        }

        ModuleStatus getStatus() {
            return _moduleStatus;
        }

        PhoneStatus getPhoneStatus() {
            return _phoneStatus;
        }

        ModuleEnableStatus getEnableStatus() {
            return _enableStatus;
        }

        bool getCallsAvailable() {
            return _callsAvailable;
        }

        bool activatePdpContext(GsmOperator gsmOperator) {
            if(!checkNetworkRegistration()) {
                return false;
            }
            if(!enableCgattCgact()) {
                return false;
            }
            setOperator(gsmOperator);
            if(!definePdpContext()) {
                return false;
            }
            return true;
        }

        bool checkNetworkRegistration() {
            auto status = sendCommand(CregReguest, CregIsOk);
            return status && checkForOk();
        }

        bool enableCgattCgact() {
            bool status = true;
            status &= sendCommand(CgattRequest, CgattIsOk);
            status &= checkForOk();

            status &= sendCommand(CgactRequest, CgactIsOk);
            status &= checkForOk();

            return status;
        }

        void setOperator(GsmOperator gsmOperator) {
            _gsmOperator = gsmOperator;
        }

        bool definePdpContext() {
            auto status = false;
            if(_gsmOperator == GsmOperator::Tele2) {
                status = sendCommand(CgdcontTele2);
            }
            return status && waitingForOk();
        }

        bool checkForOk() {
            return checkAT(AtStatusOk);
        }

        bool waitingForOk(uint32_t timeout = 100) {
            bool status = true;
            _moduleStatus = ModuleStatus::WaitingForOk;
            status &= _okRecieved.take(timeout);
            _moduleStatus = ModuleStatus::None;
            return status;
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


        GsmOperator _gsmOperator = GsmOperator::Undefined;
        DataType _gpsData;

        Threading::BinarySemaphore _readyStatusRecieved;
        Threading::BinarySemaphore _commandConfirmed;
        Threading::BinarySemaphore _dataRecieved;
        Threading::BinarySemaphore _okRecieved;

        Uarte::UarteNrf _uart;
        Gpio::IGpio& _nDisable;
        Gpio::IGpio& _nReset;

        uint8_t _gsmOperatorSelectionMode;
        bool _callsAvailable = false;
        ATS _selectedTechnology;

        bool _onCall = false;
        uint64_t _callLengthSeconds = 0;
        
        char _missedCallTime[timeSize];
        char _missedCallNumber[numberSize];

        Timer::TimerNrf52 _timer;
        size_t _bufIndex = 0;
        uint8_t _parserIndex = 0;
        std::array<uint8_t, 1> _recieveByte;
        std::array<uint8_t, 128> _recieveBuf;
        std::array<uint8_t, 128> _parseBuf;
        const char * _atCommandForCheck = nullptr;

        bool _onRecieve = false;

        bool _cPinReady = false;
        bool _smsReady = false;
        bool _pbReady = false;
        bool _gpsReady = false;
        
        bool _readyStatus = false;

        ModuleEnableStatus _enableStatus = ModuleEnableStatus::Disabled;
        ModuleStatus _moduleStatus = ModuleStatus::None;
        PhoneStatus _phoneStatus = PhoneStatus::Idle;
    };

}