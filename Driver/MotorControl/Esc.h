#pragma once

#include "DRV8328.h"
#include "AdcCh32v.h"
#include "TimerCh32.h"
#include "ThreadFreeRtos.h"
#include "Semaphore.h"
#include "TimerRtos.h"
#include "CriticalSection.h"

#include "GpioCh32v.h"

#include <cmath>

#define BR2205

namespace ES::Driver::MotorControl {

    //static constexpr uint32_t SupplyVoltage = 12000;
    //static constexpr uint32_t HighResistor = 10000;
    //static constexpr uint32_t LowResistor = 2200;

    //static constexpr uint8_t CommutationFullCircle = 42;
    //static constexpr uint32_t MinuteUs = 60'000'000;

    static  constexpr uint8_t ProcessingDelay = 10;

    static TIM_TypeDef* timMeas = TIM4;
    static TIM_TypeDef* timComm = TIM3;
    static constexpr uint32_t TimerFreqMhz = 144;
    static constexpr uint32_t Factor = 8;
    static constexpr uint32_t TimerDiv = TimerFreqMhz / Factor;

    static constexpr uint16_t Torque = static_cast<uint16_t>(Bldc::Torque::VeryLow);

#if defined(BR2205)
    static constexpr float MinThrottle = 0.2f;
    static constexpr float MaxThrottle = 0.7f;
    static constexpr float OpenThrottle = 0.4f;

#elif defined(BROTHER_HOBBY)
    static constexpr float MinThrottle = 0.3f;
    static constexpr float MaxThrottle = 0.7f;
    static constexpr float OpenLoopThrottle = 0.5f;
#else
#error
#endif
    static constexpr uint32_t AdcStop = 200;
    static constexpr uint32_t AdcStart = 700;
    static constexpr uint16_t MinimumThrottle = (Torque * MinThrottle);
    static constexpr uint16_t OpenLoopThrottle = (Torque * OpenThrottle);
    static constexpr uint16_t MaximumThrottle = (Torque * MaxThrottle);
    static constexpr uint32_t Period = (MaximumThrottle) - MinimumThrottle;
    static constexpr uint16_t AdcMax = 4000;

    static constexpr float DshotThrottleMaxValue = 2047.0f;

    //static constexpr float HalfSupplyVal = ((SupplyVoltage / 2) * LowResistor) / (HighResistor + LowResistor);
    //static constexpr uint16_t HalfSupplyAdcVal = static_cast<uint16_t>((4095.0f / 3300.0f) * HalfSupplyVal);

    //static constexpr float k = static_cast<float>(HighResistor + LowResistor) / LowResistor;
#if defined(BR2205)
    static constexpr uint32_t OpenLoopEndPeriod = 80;
    static constexpr uint32_t OpenLoopStartPeriod = 1000;
    static constexpr float OpenLoopAcceleration = 1.7f;
#elif defined(BROTHER_HOBBY)
    static constexpr uint32_t OpenLoopEndPeriod = 1000;
    static constexpr uint32_t OpenLoopStartPeriod = 20000;
    static constexpr float OpenLoopAcceleration = 1.1f;
#else
#error
#endif

    class Esc6Step {
    public:
        enum MotorState {
            Idle,
            OpenLoop,
            Started
        };

        Esc6Step(Drv8328 drv, Adc::AdcCh32vThreePhaseInj& adc, uint32_t inputVoltage) :
        _drv(drv), _adc(adc) {
            getInstance();
            updateInputVoltage(inputVoltage);
            _drv.setTorque(Torque);
            _drv.init();
            Threading::sleepForMs(1000);
            _drv.startPwm();
        }

        void parseCallback(uint8_t throttle) {
            asm("nop");
        }

        void motorStart() {
            _drv.setCompare(OpenLoopThrottle);
            _currentThrottle = OpenLoopThrottle;
            openLoopStart();
        }

        void setThrottle(float value) {
            if(_state == MotorControl::Esc6Step::MotorState::Idle) {
                _drv.setDuty(value);
            }
            else {
                _drv.setDuty(value);
            }
        }

        void setThrottleByPot(uint16_t potValue) {
            if(potValue < AdcStop) {
                return;
            }
            if(_state == MotorControl::Esc6Step::MotorState::Started) {
                uint32_t tmp = potValue * Period / (AdcMax - AdcStart);
                _drv.setCompare(static_cast<uint16_t>(tmp) + MinimumThrottle);
            }
        }

        void setThrottleByDshot(uint16_t value) {
            if(value == 0) {
                _targetThrottle = 0;
                return;
            }
            float tmp = static_cast<float>(value) * (static_cast<float>(Period) / DshotThrottleMaxValue);
            _targetThrottle = static_cast<uint16_t>(tmp) + MinimumThrottle;
            asm("nop");
        }

        void switchPhase(Bldc::Step step) {
            _drv._timComplimentary.stop();
            _drv._timComplimentary.setCounter(Torque - (Torque / 5));
            if(step == Bldc::Step::ChAl) {
                _drv.deCommutate(Bldc::MotorPhase::B);
                _drv.commutateHigh(Bldc::MotorPhase::C);
                _drv.commutateLow(Bldc::MotorPhase::A);
                _previousStep = Bldc::Step::BhAl;
                _nextStep = Bldc::Step::ChBl;
                _bemfPhase = Bldc::MotorPhase::B;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::ChBl) {

                _drv.deCommutate(Bldc::MotorPhase::A);
                _drv.commutateHigh(Bldc::MotorPhase::C);
                _drv.commutateLow(Bldc::MotorPhase::B);
                _previousStep = Bldc::Step::ChAl;
                _nextStep = Bldc::Step::AhBl;
                _bemfPhase = Bldc::MotorPhase::A;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            else if(step == Bldc::Step::AhBl) {
                _drv.deCommutate(Bldc::MotorPhase::C);
                _drv.commutateHigh(Bldc::MotorPhase::A);
                _drv.commutateLow(Bldc::MotorPhase::B);
                _previousStep = Bldc::Step::ChBl;
                _nextStep = Bldc::Step::AhCl;
                _bemfPhase = Bldc::MotorPhase::C;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::AhCl) {
                _drv.deCommutate(Bldc::MotorPhase::B);
                _drv.commutateHigh(Bldc::MotorPhase::A);
                _drv.commutateLow(Bldc::MotorPhase::C);
                _previousStep = Bldc::Step::AhBl;
                _nextStep = Bldc::Step::BhCl;
                _bemfPhase = Bldc::MotorPhase::B;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            else if(step == Bldc::Step::BhCl) {
                _drv.deCommutate(Bldc::MotorPhase::A);
                _drv.commutateHigh(Bldc::MotorPhase::B);
                _drv.commutateLow(Bldc::MotorPhase::C);
                _previousStep = Bldc::Step::AhCl;
                _nextStep = Bldc::Step::BhAl;
                _bemfPhase = Bldc::MotorPhase::A;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::BhAl) {
                _drv.deCommutate(Bldc::MotorPhase::C);
                _drv.commutateHigh(Bldc::MotorPhase::B);
                _drv.commutateLow(Bldc::MotorPhase::A);
                _previousStep = Bldc::Step::BhCl;
                _nextStep = Bldc::Step::ChAl;
                _bemfPhase = Bldc::MotorPhase::C;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            if(!_forwardDirection) {
                if(_bemfEdge == Bldc::BemfEdge::Rising) {
                    _bemfEdge = Bldc::BemfEdge::Falling;
                }
                else {
                    _bemfEdge = Bldc::BemfEdge::Rising;
                }
            }
            _adc.setMaxSampleTime(_bemfPhase);
            _drv._timComplimentary.start();
        }

        void nextStep() {
            if(_forwardDirection) {
                switchPhase(_nextStep);
            }
            else {
                switchPhase(_previousStep);
            }
        }

        void intTim3Callback() {
            _drv._timComplimentary.triggerDisable();
            nextStep();
            _measureTimer.stop();
            _measureTimer.setCounter(0);
            _measureTimer.start();
            _adcFlag = true;
            _stepCommutationTimer.stop();
            _stepCommutationTimer.setCounter(0);
        }

        void intTim1Callback() {
            _drv._timComplimentary.triggerEn();
            _counter = _measureTimer.getCounter();
            _currentSwitchPeriod = _counter;
        }

        void intTim4Callback() {
            motorStop();
            motorStart();
        }

        void updateInputVoltage(uint32_t adcValue) {
            _inputVoltage = _adc.getVoltageMv<10000, 2200>(adcValue);
            _halfSupplyAdcValue = adcValue / 2;
        }

        void motorStop() {
            _measureTimer.stop();
            _measureTimer.setCounter(0);
            _stepCommutationTimer.stop();
            _stepCommutationTimer.setCounter(0);
            _drv.deCommutate(Bldc::MotorPhase::A);
            _drv.deCommutate(Bldc::MotorPhase::B);
            _drv.deCommutate(Bldc::MotorPhase::C);
            _adc.disable();
            _state = MotorState::Idle;
            _currentThrottle = 0;
            _drv.setCompare(_currentThrottle);
            //Threading::sleepForMs(1000);
        }

        void reverseDirection() {
            motorStop();
            Threading::sleepForMs(1000);
            _forwardDirection = !_forwardDirection;
            motorStart();
        }

        uint32_t getSpeedRpm() {
            return _rpm;
        }

        MotorState getState() {
            return _state;
        }

    private:

        Threading::Thread _speedControlHandle{"speedControl", 256, Threading::ThreadPriority::Normal,  [this](){
            uint16_t diff = 0;
            while(true) {
                if(_targetThrottle > _currentThrottle) {
                    if(_state == MotorState::Idle) {
                        motorStart();
                    }
                    if(_state == MotorState::Started) {
                        diff = _targetThrottle - _currentThrottle;
                        diff = diff * 0.1f;
                        if(diff < 5) {
                            diff = 4;
                        }
                        _currentThrottle = _currentThrottle + diff;
                        _drv.setCompare(_currentThrottle);
                    }
                }
                else if(_targetThrottle < _currentThrottle) {
                    if(_state != MotorState::Idle) {
                        diff = _currentThrottle - _targetThrottle;
                        diff = diff * 0.1f;
                        if(diff < 5) {
                            diff = 4;
                        }
                        _currentThrottle = _currentThrottle - diff;
                        _drv.setCompare(_currentThrottle);

                        if(_targetThrottle == 0) {
                            _currentThrottle = 0;
                            motorStop();
                            Threading::sleepForMs(500);
                        }
                    }
                }

                Threading::sleepForMs(10);
            }
        }};

        Threading::Thread _motorTorqueHandle{"MotorTorqueControl", 256, Threading::ThreadPriority::Normal,  [this](){
            uint16_t counter = 0;

            bool bemfFlag = false;

            uint8_t index = 0;
            size_t currentSize = 0;
            uint32_t sum = 0;

            while(true) {
                while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC)) {
                    Threading::yield();
                }
                ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);

                Threading::CriticalSection _lock{};

                //adcValueA = ADC1->IDATAR1;
                //adcValueB = ADC1->IDATAR2;
                //adcValueC = ADC1->IDATAR3;

                if(_bemfPhase == Bldc::MotorPhase::A) { 
                    _adcValue = ADC1->IDATAR1;
                }
                if(_bemfPhase == Bldc::MotorPhase::B) { 
                    _adcValue = ADC1->IDATAR2;
                }
                if(_bemfPhase == Bldc::MotorPhase::C) { 
                    _adcValue = ADC1->IDATAR3;
                }
                if(_adcFlag) {
                    if(_bemfEdge == Bldc::BemfEdge::Falling) {
                        bemfFlag = _adcValue < _halfSupplyAdcValue;
                    }
                    else if(_bemfEdge == Bldc::BemfEdge::Rising) {
                        bemfFlag = _adcValue > _halfSupplyAdcValue;
                    }
                    if(bemfFlag) {
                        _measureTimer.stop();
                        _measureTimer.setCounter(0);
                        _measureTimer.start();

                        _adcFlag = false;
                        //counter = _measureTimer.getCounter() - ProcessingDelay;

                        counter = _counter;
                        /*sum = 0;
                        buf[index] = static_cast<uint16_t>(counter);
                        index++;
                        if(index == windowSize) {
                            index = 0;
                        }
                        if(currentSize != windowSize) {
                            currentSize++;
                        }
                        for(uint8_t i = 0; i < currentSize; i++)  {
                            sum += buf[i];
                        }
                        sum /= currentSize;
                        _currentSwitchPeriod = sum * 2;
                        _stepCommutationTimer.setPeriod(sum);*/
                        _stepCommutationTimer.setPeriod(counter);
                        _stepCommutationTimer.start();

                        /*if(!_stopWatch.isStarted()) {
                            _stopWatch.start();
                            _stepFullCircleCount++;
                        }
                        else if(_stepFullCircleCount == CommutationFullCircle) {
                            _stepFullCircleCount = 0;
                            _stopWatch.stop();
                        }
                        else {
                            _stepFullCircleCount++;
                        }*/
                    }
                }
            }
        }};

        Threading::Thread _motorStartHandle{"MotorStartControl", 256, Threading::ThreadPriority::Normal,  [this](){
            uint32_t currentPeriod = 0;
            while(true) {
                if(_openLoopStart) {
                    //Threading::sleepForMs(100);
                    _openLoopStart = false;
                    nextStep();
                    Threading::sleepForMs(5);

                    currentPeriod = OpenLoopStartPeriod;
                    while(currentPeriod > OpenLoopEndPeriod) {
                        Threading::sleepForUs(currentPeriod);
                        currentPeriod /= OpenLoopAcceleration;
                        nextStep();
                    }
                    _measureTimer.setIrq(0);
                    _measureTimer.start();

                    _stepCommutationTimer.setIrq(0);
                    _stepCommutationTimer.setPeriod(OpenLoopEndPeriod  * TimerDiv);
                    _stepCommutationTimer.start();
                    _adc.enable();
                    _drv._timComplimentary.setIrq(0);
                    _targetThrottle = MinimumThrottle;
                    //_drv.setCompare(MinimumThrottle);
                    _state = MotorState::Started;
                }
                Threading::sleepForMs(100);
            }
        }};

        void openLoopStart() {
            _state = MotorState::OpenLoop;
            _openLoopStart = true;
        }

        MotorState _state = MotorState::Idle;
        uint16_t _currentThrottle = 0;
        uint16_t _targetThrottle = _currentThrottle;

        Bldc::Step _currentStep;
        Bldc::Step _previousStep = static_cast<Bldc::Step>(6);
        Bldc::Step _nextStep = static_cast<Bldc::Step>(1);
        Bldc::MotorPhase _bemfPhase;
        Bldc::BemfEdge _bemfEdge;

        Threading::StopWatch _stopWatch{};

        bool _forwardDirection = false;

        uint32_t _rpm = 0;
        uint32_t _currentSwitchPeriod = 0;
        uint32_t _stepFullCircleCount = 0;

        Threading::BinarySemaphore _tim2Event;
        bool _openLoopStart;

        float _inputVoltage = 0; 
        uint32_t _halfSupplyAdcValue = 0;

        void getInstance();
        bool _adcFlag = false;

        uint16_t _counter = 0;

        static const size_t windowSize = 6;
        uint16_t buf[windowSize];

        Drv8328 _drv;
        Adc::AdcCh32vThreePhaseInj& _adc;
        uint16_t _adcValue;

        Timer::TimerBaseCh32v _measureTimer = {timMeas, 0xFFFF, TimerDiv};
        Timer::TimerBaseCh32v _stepCommutationTimer = {timComm, 0xFFFF, TimerDiv};
        
    };

}