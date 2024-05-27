#pragma once
#include "SEGGER_RTT.h"
#include "UarteNrf52.h"
#include "IGpio.h"
#include "TimerNrf52.h"

//#include "FreeRTOS.h"
#include "CriticalSection.h"
#include "Semaphore.h"
#include "ThreadFreeRtos.h"

#include <array>
#include <string>
#include <cmath>
#include <functional>

#include "nrf_check_ret.h"
#include "CommonTools.h"
#include "SIM7600AtCommands.h"
#include "SIMTypes.h"
#include "PPP.h"

#include "PinMap1.0.h"

namespace ES::Driver {
  
    static constexpr size_t TimeSize = 7;
    static constexpr size_t NumberSize = 12;

    template<size_t bufferSize>
    class Sim7600x {
    public:

        Sim7600x(Gpio::IGpio& nDisable, Gpio::IGpio& nReset, std::function<bool(char*, size_t)>& writeStream) : _writeStream(writeStream), _nDisable(nDisable), _nReset(nReset)
        {   
            _nDisable.configureOutput();
            _nDisable.set();
            _nReset.configureOutput();
            _nReset.set();//default power off
            _enableStatus = ModuleEnableStatus::Disabled;
        }

        //Sim7600x(Uarte::UarteNrf uart, Timer::TimerNrf52 timer, Gpio::IGpio& nDisable/*, Gpio::Nrf52Gpio nReset, Gpio::Nrf52Gpio levelConvEn, Gpio::Nrf52Gpio modulePowerEn, Gpio::Nrf52Gpio ldo1V8En*/) : _uart(uart), _nDisable(nDisable), _timer(timer)/*, _nReset(nReset), _levelConvEn(levelConvEn), _modulePowerEn(modulePowerEn), _ldo1V8En(ldo1V8En) */{}

        void enableModule() { 
            _nReset.reset(); //power on
            _nDisable.set();
            _enableStatus = ModuleEnableStatus::Disabled;
            // while(_enableStatus != ModuleEnableStatus::Enabled) {
            //     Threading::sleepForMs(100);
            // }
        }

        void disableModule() {
            _nDisable.reset(); //TODO invert
            Threading::sleepForMs(50);
        }

        void resetModule() {
            _nReset.set();
            Threading::sleepForMs(100);
            _nReset.reset();
        }

        bool sendCommand(const char * s, const char * expectedAnswer = nullptr) {
            bool status = sendString(s);
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
 
        ret_code_t sendString(const char * s) {
            ret_code_t status;
            size_t size2 = CommonTools::charArraySize(s);
            char tempBuf[128];
            //Use only CR
            memcpy(&tempBuf[0], s, size2);
            memcpy(&tempBuf[size2], AtCrLf, 1);
            status = _writeStream(tempBuf, size2 + 1);
            return CheckErrorCode::success(status);
        }

        ret_code_t sendData(const char * s, size_t arraySize) {
            ret_code_t status;
            unsigned char tempBuf[128];
            memcpy(tempBuf, s, arraySize);
            status = _writeStream(reinterpret_cast<char*>(tempBuf), arraySize);
            //status =_uart.writeStream(reinterpret_cast<char*>(tempBuf), arraySize);
            return CheckErrorCode::success(status);
        }

        ret_code_t sendCrLf() {
            char tempBuf[2] = {AtCrLf[0], AtCrLf[1]};
            return _writeStream(tempBuf, 2);
        }

        void atRecieve(typename std::array<uint8_t, bufferSize>::const_iterator it, size_t messageSize) {
            std::fill(_parseBuf.begin(), _parseBuf.end(), 0);
            {  
                Threading::CriticalSection lock;
                std::copy(it, it + messageSize, std::begin(_parseBuf));
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

        bool checkAT(const char* s) {
            bool status = true;
            size_t stringSize = CommonTools::charArraySize(s) + _parserIndex;
            skipCrLf();
            for(uint8_t i = 0; _parserIndex < stringSize; _parserIndex++, i++) {
                if(s[i] == '\0') {
                    break;
                }
                if(s[i] != static_cast<char>(_parseBuf[_parserIndex])) {
                    return false;
                }
            }
            _parserIndex++;
            skipCrLf();
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
            if(_enableStatus == ModuleEnableStatus::Disabled) {
                return false; //modele waiting ready status. It has no time for GPS
            }
            if(_moduleStatus == ModuleStatus::None && !_gpsReady) {
                status = sendString(AtCommandGpsEnable);
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

        bool gpsReady() {
            return _gpsReady;
        }

        bool getLocation() {
            if(!_gpsReady) {
                return false;
            }
            ret_code_t status;
            if(_moduleStatus == ModuleStatus::None) {
                status = sendString(AtCommandGpsInfo);
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
                while(index < TimeSize) {
                    _parserIndex++;
                    _missedCallTime[index] = _parseBuf[_parserIndex];
                    index++;
                }
                _parserIndex++;
                index = 0;
                while(index < NumberSize) {
                    _parserIndex++;
                    _missedCallNumber[index] = _parseBuf[_parserIndex];
                    index++;
                }
                _parserIndex = 0;
            }
            if(dataType == DataType::ConnectionCompleteData) {
                if(checkAT(ConnectionPppMode)) {
                    _modemMode = ModemMode::DataMode;
                    return true;
                }
                else {
                    return false;
                }
            }
            return true;
        }

        bool makeCall(const char * number) {
            char atdCommand[17];
            strcpy(atdCommand, MakeCall);
            strcpy((atdCommand + CommonTools::charArraySize(MakeCall)), number);
            auto status = sendCommand(atdCommand);
            if(!status) {
                return false;
            }
            status &= waitingForOk();
            if(status) {
                _phoneStatus = PhoneStatus::OutgoingPreCall;
            }
            return status;
        }

        bool setAthAvilable() {
            auto status = sendCommand(SetAthAvilable);
            return status && checkForOk();
        }

        bool enableLinePresentation() {
            auto status = sendCommand(SetLinePresentation);
            return status && checkForOk();
        }

        bool disconnectCall() {
            bool status;
            if(_phoneStatus == PhoneStatus::IncomingPreCall || _phoneStatus == PhoneStatus::IncomingCall) {
                status = sendCommand(DisconnectCall);
            }
            else {
                status = sendCommand(HangUp);
            }
            status &= waitingForOk();
            if(status) {
                _phoneStatus = PhoneStatus::Idle;
            }
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

        bool switchAuthToPap() {
            auto status = sendCommand(CgAuthPap, CgAuthPap);
            return status && checkForOk();
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

        Threading::BinarySemaphore incomingCall;
        
        bool testCall() {
            return sendCommand(AtdTest);
        }

        bool enableCreg() {
            return sendCommand(CregEnable);
        }

        bool checkCreg() {
            return sendCommand(CregReguest);
        }

        bool ckeckCops() {
            return sendCommand(AtCopsRead);
        }

        bool setCmnp(uint8_t value) {
            if(value == 1) {
                sendCommand(CmnpGsmLteOnly);
            }
            if(value == 2) {
                sendCommand(CmnpLteOnly);
            }
            if(value == 3) {
                sendCommand(CmnpGsmOnly);
            }
            if(value == 4) {
                sendCommand(CmnpAutoOnly);
            }
            return true;
        }        

        bool checkCsq() {
            return sendCommand(CheckCsq);
        }

        bool switchToDataMode() {
            bool status = true;
            status &= sendCommand(CgData);
            _moduleStatus = ModuleStatus::WaitingForData;
            if(_dataRecieved.take(1000)) {
                status &= parseData(DataType::ConnectionCompleteData);
                
            }
            return status;
        }

        ModemMode getModemMode() {
            return _modemMode;
        }

    private:

        std::function<bool(char*, size_t)>& _writeStream;
        ModemMode _modemMode = ModemMode::CommandMode;

        GsmOperator _gsmOperator = GsmOperator::Undefined;
        DataType _gpsData;

        Threading::BinarySemaphore _readyStatusRecieved;
        Threading::BinarySemaphore _commandConfirmed;
        Threading::BinarySemaphore _dataRecieved;
        Threading::BinarySemaphore _okRecieved;

        Gpio::IGpio& _nDisable;
        Gpio::IGpio& _nReset;

        uint8_t _gsmOperatorSelectionMode;
        bool _callsAvailable = false;
        ATS _selectedTechnology;

        bool _onCall = false;
        uint64_t _callLengthSeconds = 0;
        
        char _missedCallTime[TimeSize];
        char _missedCallNumber[NumberSize];

        uint8_t _parserIndex = 0;
        std::array<uint8_t, bufferSize> _parseBuf;
        const char * _atCommandForCheck = nullptr;

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
