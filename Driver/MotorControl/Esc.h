#pragma once

#include "DRV8328.h"
#include "AdcCh32v.h"
#include "TimerCh32.h"
#include "ThreadFreeRtos.h"
#include "Semaphore.h"

#include "GpioCh32v.h"

#include <cmath>

namespace ES::Driver::MotorControl {

    static constexpr uint32_t SupplyVoltage = 12000;
    static constexpr uint32_t HighResistor = 10000;
    static constexpr uint32_t LowResistor = 2200;

    static constexpr uint8_t CommutationFullCircle = 42;

    static  constexpr uint8_t ProcessingDelay = 10;

    static TIM_TypeDef* timMeas = TIM2;
    static TIM_TypeDef* timComm = TIM3;
    static constexpr uint32_t TimerFreqMhz = 144;
    static constexpr uint32_t TimerDiv = 4;

    static constexpr float HalfSupplyVal = ((SupplyVoltage / 2) * LowResistor) / (HighResistor + LowResistor);
    static constexpr uint16_t HalfSupplyAdcVal = static_cast<uint16_t>((4095.0f / 3300.0f) * HalfSupplyVal);

    static constexpr float k = static_cast<float>(HighResistor + LowResistor) / LowResistor;

    static constexpr uint32_t OpenLoopEndPeriod = 2000;
    static constexpr uint32_t OpenLoopStartPeriod = 50000;
    static constexpr float OpenLoopAcceleration = 1.5f;

    class Esc6Step {
    public:
        Esc6Step(Drv8328 drv, Adc::AdcCh32vThreePhaseInj& adc, uint32_t bemfThreshold, Gpio::Ch32vPin& testGpio) :
        _drv(drv), _adc(adc), _testGpio(testGpio) {
            getInstance();
            updateBemfThreshold(bemfThreshold);
            _drv.init();
            Threading::sleepForMs(1000);
            _drv.startPwm();
        }

        void motorStart() {
            openLoopStart();
        }

        void switchPhase(Bldc::Step step) {
            _currentStep = step;
            if(step == Bldc::Step::ChAl) {
                _drv._timComplimentary.stop();
                _drv._timComplimentary.setCounter(0);
                _drv.deCommutate(Bldc::MotorPhase::B);
                _drv.commutateHigh(Bldc::MotorPhase::C);
                _drv.commutateLow(Bldc::MotorPhase::A);
                _drv._timComplimentary.start();
                _previousStep = Bldc::Step::BhAl;
                _nextStep = Bldc::Step::ChBl;
                _bemfPhase = Bldc::MotorPhase::B;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::ChBl) {
                _drv._timComplimentary.stop();
                _drv._timComplimentary.setCounter(0);
                _drv.deCommutate(Bldc::MotorPhase::A);
                _drv.commutateHigh(Bldc::MotorPhase::C);
                _drv.commutateLow(Bldc::MotorPhase::B);
                _drv._timComplimentary.start();
                _previousStep = Bldc::Step::ChAl;
                _nextStep = Bldc::Step::AhBl;
                _bemfPhase = Bldc::MotorPhase::A;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            else if(step == Bldc::Step::AhBl) {
                _drv._timComplimentary.stop();
                _drv._timComplimentary.setCounter(0);
                _drv.deCommutate(Bldc::MotorPhase::C);
                _drv.commutateHigh(Bldc::MotorPhase::A);
                _drv.commutateLow(Bldc::MotorPhase::B);
                _drv._timComplimentary.start();
                _previousStep = Bldc::Step::ChBl;
                _nextStep = Bldc::Step::AhCl;
                _bemfPhase = Bldc::MotorPhase::C;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::AhCl) {
                _drv._timComplimentary.stop();
                _drv._timComplimentary.setCounter(0);
                _drv.deCommutate(Bldc::MotorPhase::B);
                _drv.commutateHigh(Bldc::MotorPhase::A);
                _drv.commutateLow(Bldc::MotorPhase::C);
                _drv._timComplimentary.start();
                _previousStep = Bldc::Step::AhBl;
                _nextStep = Bldc::Step::BhCl;
                _bemfPhase = Bldc::MotorPhase::B;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            else if(step == Bldc::Step::BhCl) {
                _drv._timComplimentary.stop();
                _drv._timComplimentary.setCounter(0);
                _drv.deCommutate(Bldc::MotorPhase::A);
                _drv.commutateHigh(Bldc::MotorPhase::B);
                _drv.commutateLow(Bldc::MotorPhase::C);
                _drv._timComplimentary.start();
                _previousStep = Bldc::Step::AhCl;
                _nextStep = Bldc::Step::BhAl;
                _bemfPhase = Bldc::MotorPhase::A;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::BhAl) {
                _drv._timComplimentary.stop();
                _drv._timComplimentary.setCounter(0);
                _drv.deCommutate(Bldc::MotorPhase::C);
                _drv.commutateHigh(Bldc::MotorPhase::B);
                _drv.commutateLow(Bldc::MotorPhase::A);
                _drv._timComplimentary.start();
                _previousStep = Bldc::Step::BhCl;
                _nextStep = Bldc::Step::ChAl;
                _bemfPhase = Bldc::MotorPhase::C;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
        }

        void nextStep() {
            switchPhase(_nextStep);
        }

        void prevStep() {
            switchPhase(_previousStep);
        }

        void intTim3Callback() {
            _drv._timComplimentary.triggerDisable();
            nextStep();
            _measureTimer.start();
            _adcFlag = true;
            _stepCommutationTimer.stop();
            _stepCommutationTimer.setCounter(0);
        }

        void intTim1Callback() {
            _drv._timComplimentary.triggerEn();
            _counter = _measureTimer.getCounter();
        }

        void intTim2Callback() {
            _measureTimer.stop();
            _measureTimer.setCounter(0);
            _stepCommutationTimer.stop();
            _stepCommutationTimer.setCounter(0);
            _adc.disable();
            _openLoopStart.give();
        }

        void updateBemfThreshold(uint32_t adcValue) {
            _halfSupplyAdcValue = adcValue / 2;
        }

    private:

        Threading::Thread _motorTorqueHandle{"MotorTorqueControl", 256, Threading::ThreadPriority::Normal,  [this](){
            uint16_t counter = 0;

            bool bemfFlag = false;

            uint16_t adcValueA;
            uint16_t adcValueB;
            uint16_t adcValueC;

            uint8_t index = 0;
            size_t currentSize = 0;
            uint32_t sum = 0;

            while(true) {
                while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC)) {
                    Threading::yield();
                }
                ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);

                adcValueA = ADC1->IDATAR1;
                adcValueB = ADC1->IDATAR2;
                adcValueC = ADC1->IDATAR3;

                if(_bemfPhase == Bldc::MotorPhase::A) { 
                    _adcValue = adcValueA;
                }
                if(_bemfPhase == Bldc::MotorPhase::B) { 
                    _adcValue = adcValueB;
                }
                if(_bemfPhase == Bldc::MotorPhase::C) { 
                    _adcValue = adcValueC;
                }
                if(_adcFlag) {
                    if(_bemfEdge == Bldc::BemfEdge::Falling) {
                        bemfFlag = _adcValue < _halfSupplyAdcValue;
                    }
                    else if(_bemfEdge == Bldc::BemfEdge::Rising) {
                        bemfFlag = _adcValue > _halfSupplyAdcValue;
                    }
                    if(bemfFlag) {
                        _adcFlag = false;

                        _measureTimer.stop();
                        //counter = _measureTimer.getCounter() - ProcessingDelay;
                        counter = _counter;
                        _measureTimer.setCounter(0);

                        sum = 0;
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

                        _stepCommutationTimer.setPeriod(sum);
                        _stepCommutationTimer.start();
                    }
                }
            }
        }};

        Threading::Thread _motorStartHandle{"MotorStartControl", 256, Threading::ThreadPriority::Normal,  [this](){
            uint32_t currentPeriod = 0;
            while(true) {
                _openLoopStart.take();
                Threading::sleepForMs(1000);
                nextStep();
                Threading::sleepForMs(20);

                currentPeriod = OpenLoopStartPeriod;
                while(currentPeriod > OpenLoopEndPeriod) {
                    Threading::sleepForUs(currentPeriod);
                    currentPeriod /= OpenLoopAcceleration;
                    nextStep();
                }

                _stepCommutationTimer.setIrq(0);
                _stepCommutationTimer.setPeriod(OpenLoopEndPeriod);
                _stepCommutationTimer.start();
                _adc.enable();
                _drv._timComplimentary.setIrq(0);
                _measureTimer.setIrq(0);
            }
        }};

        void openLoopStart() {
            _openLoopStart.give();
        }

        Bldc::Step _currentStep;
        Bldc::Step _previousStep = static_cast<Bldc::Step>(6);
        Bldc::Step _nextStep = static_cast<Bldc::Step>(1);
        Bldc::MotorPhase _bemfPhase;
        Bldc::BemfEdge _bemfEdge;

        Threading::BinarySemaphore _tim3Event;
        Threading::BinarySemaphore _openLoopStart;

        uint32_t _halfSupplyAdcValue = 0;

        void getInstance();
        bool _adcFlag = false;

        uint16_t _counter = 0;

        static const size_t windowSize = CommutationFullCircle;
        uint16_t buf[windowSize];

        Drv8328 _drv;
        Adc::AdcCh32vThreePhaseInj& _adc;
        uint16_t _adcValue;

        Timer::TimerBaseCh32v _measureTimer = {timMeas, 0xFFFF, TimerFreqMhz / TimerDiv};
        Timer::TimerBaseCh32v _stepCommutationTimer = {timComm, 0xFFFF, TimerFreqMhz / TimerDiv};

        Gpio::Ch32vPin& _testGpio;
        
    };

}