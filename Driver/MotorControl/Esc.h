#pragma once

#include "DRV8328.h"
#include "AdcCh32v.h"
#include "TimerCh32.h"
#include "ThreadFreeRtos.h"
#include "Semaphore.h"

#include "GpioCh32v.h"

#include <cmath>

namespace ES::Driver::MotorControl {

    static constexpr uint32_t supplyVoltage = 12000;
    static constexpr uint32_t highResistor = 10000;
    static constexpr uint32_t lowResistor = 2200;

    static constexpr uint8_t commutationFullCircle = 42;
    static constexpr uint32_t minuteUs = 60000000;

    static TIM_TypeDef* timMeas = TIM2;
    static TIM_TypeDef* timComm = TIM3;

    static constexpr float halfSupplyVal = ((supplyVoltage / 2) * lowResistor) / (highResistor + lowResistor);
    static constexpr uint16_t halfSupplyAdcVal = static_cast<uint16_t>((4095.0f / 3300.0f) * halfSupplyVal);

    class Esc6Step {
    public:
        Esc6Step(Drv8328 drv, Adc::AdcCh32vThreePhaseInj& adc, Gpio::Ch32vPin& testGpio) :
        _drv(drv), _adc(adc), _testGpio(testGpio) {
            getInstance();
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

        Bldc::Step _currentStep;
        Bldc::Step _previousStep = static_cast<Bldc::Step>(6);
        Bldc::Step _nextStep = static_cast<Bldc::Step>(1);
        Bldc::MotorPhase _bemfPhase;
        Bldc::BemfEdge _bemfEdge;
        
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
        }

    private:



        Threading::Thread _motorMeasurementsHandle{"MotorMeasure", 512, Threading::ThreadPriority::Normal,  [this](){
            _openLoopStart.take();
            Threading::sleepForMs(5000);
            nextStep();
            Threading::sleepForMs(50);

            
            _currentPeriodUs = _openLoopStartPeriodUs;
            while(_currentPeriodUs > _openLoopEndPeriodUs) {
                Threading::sleepForUs(_currentPeriodUs);
                _currentPeriodUs /= 1.8f;
                nextStep();
            }

            _stepCommutationTimer.setIrq(0);
            _stepCommutationTimer.setPeriod(_openLoopEndPeriodUs);
            _currentSpeed = _openLoopEndPeriodUs;
            _stepCommutationTimer.start();
            _adc.enable();
            _drv._timComplimentary.setIrq(0);

            _openLoopEnd.give();
                       
            while(true) {
                while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC)) {
                    Threading::yield();
                }
                ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);

                _adcValueA = ADC1->IDATAR1;
                _adcValueB = ADC1->IDATAR2;
                _adcValueC = ADC1->IDATAR3;

                if(_bemfPhase == Bldc::MotorPhase::A) { 
                    _adcValue = _adcValueA;
                }
                if(_bemfPhase == Bldc::MotorPhase::B) { 
                    _adcValue = _adcValueB;
                }
                if(_bemfPhase == Bldc::MotorPhase::C) { 
                    _adcValue = _adcValueC;
                }
                bool bemfFlag = false;
                if(_adcFlag) {
                    if(_bemfEdge == Bldc::BemfEdge::Falling) {
                        bemfFlag = _adcValue < halfSupplyAdcVal;
                    }
                    else if(_bemfEdge == Bldc::BemfEdge::Rising) {
                        bemfFlag = _adcValue > halfSupplyAdcVal;
                    }
                    if(bemfFlag) {
                        _adcFlag = false;

                        _measureTimer.stop();
                        int32_t counter = _measureTimer.getCounter();
                        _measureTimer.setCounter(0);

                        _sum = 0;
                        buf[index] = static_cast<uint16_t>(counter);
                        index++;
                        if(index == windowSize) {
                            index = 0;
                        }
                        if(currentSize != windowSize) {
                            currentSize++;
                        }
                        for(uint8_t i = 0; i < currentSize; i++)  {
                            _sum += buf[i];
                        }
                        _sum /= currentSize;

                        _stepCommutationTimer.setPeriod(_sum);
                        _stepCommutationTimer.start();
                    }
                }

            }
        }};

        Threading::Thread _motorControl{"MotorControl", 512, Threading::ThreadPriority::Normal,  [this](){
            _openLoopEnd.take();
            while(true) {
                if(_sum != 0) {
                    /*_spinsPerMinuteNew = minuteUs / (_sum * 2 * commutationFullCircle);
                    if(abs(_spinsPerMinuteNew - _spinsPerMinuteCurrent) > 1000) {
                        _spinsPerMinuteCurrent = _spinsPerMinuteNew;
                        _currentSpeed = _sum;
                    }*/
                    //_currentSpeed = _sum;
                }
                Threading::sleepForMs(20);
            }

        }};

        void openLoopStart() {
            _openLoopStart.give();
        }

        Threading::BinarySemaphore _tim3Event;
        Threading::BinarySemaphore _openLoopStart;
        Threading::BinarySemaphore _openLoopEnd;

        void getInstance();
        bool _adcFlag = false;
        bool _openLoop = false;
        static constexpr uint32_t _openLoopEndPeriodUs = 1000;
        static constexpr uint32_t _openLoopStartPeriodUs = 20000;
        int _spinsPerMinuteNew = 0;
        int _spinsPerMinuteCurrent = 0;

        uint32_t _currentSpeed = 0;

        static const size_t windowSize = commutationFullCircle;
        uint16_t buf[windowSize];
        uint8_t index = 0;
        size_t currentSize = 0;

        uint32_t _sum = 0;
        uint32_t _currentPeriodUs = 0;

        Drv8328 _drv;
        Adc::AdcCh32vThreePhaseInj& _adc;
        uint16_t _adcValue;

        uint16_t _adcValueA;
        uint16_t _adcValueB;
        uint16_t _adcValueC;

        Timer::TimerBaseCh32v _measureTimer = {timMeas, 0xFFFF, 144};
        Timer::TimerBaseCh32v _stepCommutationTimer = {timComm, 0xFFFF, 144};

        Gpio::Ch32vPin& _testGpio;
    };

}