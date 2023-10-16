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
            Threading::sleepForMs(1000000);
            _drv.startPwm();
        }

        void motorStart() {
            openLoopStart();
        }

        void switchPhase(Bldc::Step step) {
            _currentStep = step;
            if(step == Bldc::Step::ChAl) {
                _drv._timComplimentary.stop();
                _drv.deCommutate(Bldc::MotorPhase::B);
                _drv.commutateHigh(Bldc::MotorPhase::C);
                _drv.commutateLow(Bldc::MotorPhase::A);
                _drv._timComplimentary.start();
                _nextStep = Bldc::Step::ChBl;
                _bemfPhase = Bldc::MotorPhase::B;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::ChBl) {
                _drv._timComplimentary.stop();
                _drv.deCommutate(Bldc::MotorPhase::A);
                _drv.commutateHigh(Bldc::MotorPhase::C);
                _drv.commutateLow(Bldc::MotorPhase::B);
                _drv._timComplimentary.start();
                _nextStep = Bldc::Step::AhBl;
                _bemfPhase = Bldc::MotorPhase::A;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            else if(step == Bldc::Step::AhBl) {
                _drv._timComplimentary.stop();
                _drv.deCommutate(Bldc::MotorPhase::C);
                _drv.commutateHigh(Bldc::MotorPhase::A);
                _drv.commutateLow(Bldc::MotorPhase::B);
                _drv._timComplimentary.start();
                _nextStep = Bldc::Step::AhCl;
                _bemfPhase = Bldc::MotorPhase::C;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::AhCl) {
                _drv._timComplimentary.stop();
                _drv.deCommutate(Bldc::MotorPhase::B);
                _drv.commutateHigh(Bldc::MotorPhase::A);
                _drv.commutateLow(Bldc::MotorPhase::C);
                _drv._timComplimentary.start();
                _nextStep = Bldc::Step::BhCl;
                _bemfPhase = Bldc::MotorPhase::B;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            else if(step == Bldc::Step::BhCl) {
                _drv._timComplimentary.stop();
                _drv.deCommutate(Bldc::MotorPhase::A);
                _drv.commutateHigh(Bldc::MotorPhase::B);
                _drv.commutateLow(Bldc::MotorPhase::C);
                _drv._timComplimentary.start();
                _nextStep = Bldc::Step::BhAl;
                _bemfPhase = Bldc::MotorPhase::A;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::BhAl) {
                _drv._timComplimentary.stop();
                _drv.deCommutate(Bldc::MotorPhase::C);
                _drv.commutateHigh(Bldc::MotorPhase::B);
                _drv.commutateLow(Bldc::MotorPhase::A);
                _drv._timComplimentary.start();
                _nextStep = Bldc::Step::ChAl;
                _bemfPhase = Bldc::MotorPhase::C;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
        }

        void nextStep() {
            uint8_t stepNumber = static_cast<uint8_t>(_currentStep);
            if(stepNumber == 6) {
                stepNumber = 1;
            }
            else {
                stepNumber++;
            }
            _currentStep = static_cast<Bldc::Step>(stepNumber);
            switchPhase(_currentStep);
        }

        Bldc::Step _currentStep = static_cast<Bldc::Step>(1);
        Bldc::Step _nextStep;
        Bldc::MotorPhase _bemfPhase;
        Bldc::BemfEdge _bemfEdge;
        
        void intTim3Callback() {
            TIM_ClearITPendingBit(timComm, TIM_IT_Update);
            _drv._timComplimentary.triggerDisable();
            nextStep();
            _measureTimer.start();
            _adcFlag = true;
            _stepCommutationTimer.stop();
        }

        void intTim1Callback() {
            _drv._timComplimentary.triggerEn();
            _testGpio.set();
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            asm("nop");
            _testGpio.reset();
        }

    private:

        Threading::Thread _motorControlHandle{"MotorControl", 512, Threading::ThreadPriority::Normal,  [this](){
            _openLoopStart.take();
            _measureTimer.stop();
            _measureTimer.setCounter(0);
            _stepCommutationTimer.stop();
            //_stepCommutationTimer.setIrq(0);
            vTaskDelay(5000000);
            nextStep();
            vTaskDelay(200000);

            _stepCommutationTimer.setIrq(0);
            for(int temp = 6; temp != 0; temp--) {
                nextStep();
                vTaskDelay(100000);
            }
                for(int temp = 6; temp != 0; temp--) {
                nextStep();
                vTaskDelay(50000);
            }
                for(int temp = 6; temp != 0; temp--) {
                nextStep();
                vTaskDelay(30000);
            }
                for(int temp = 6; temp != 0; temp--) {
                nextStep();
                vTaskDelay(10000);
            }
                for(int temp = 6; temp != 0; temp--) {
                nextStep();
                vTaskDelay(5000);
            }

            for(int temp = 200; temp != 0; temp--) {
                nextStep();
                vTaskDelay(2500);
            }

            
            for(int temp = 500; temp != 0; temp--) {
                nextStep();
                vTaskDelay(2000);
            }

            for(int temp = 500; temp != 0; temp--) {
                nextStep();
                vTaskDelay(1700);
            }

            for(int temp = 500; temp != 0; temp--) {
                nextStep();
                vTaskDelay(1500);
            }

            /*for(int temp = 500; temp != 0; temp--) {
                nextStep();
                vTaskDelay(1300);
            }*/

            /*for(int temp = 500; temp != 0; temp--) {
                nextStep();
                vTaskDelay(1100);
            }*/

            /*for(int temp = 500; temp != 0; temp--) {
                nextStep();
                vTaskDelay(900);
            }*/

            int delay = 1800;
            nextStep();
            _measureTimer.stop();
            _measureTimer.setCounter(0);
            _measureTimer.start();
            //_stepCommutationTimer.start();
            _adc.enable();
            _drv._timComplimentary.setIrq(0);
            _adcFlag = true;

            
            //TIM3->ATRLR = delay;
                       
            while(true) {
                while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_JEOC)) {
                    Threading::yield();
                }
                ADC_ClearFlag(ADC1, ADC_FLAG_JEOC);
                while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {
                    Threading::yield();
                }
                ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

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
                if(_adcFlag) {
                    if(_bemfEdge == Bldc::BemfEdge::Falling) {
                        if(_adcValue < halfSupplyAdcVal) {
                            _measureTimer.stop();
                            _stepCommutationTimer.stop();
                            _stepCommutationTimer.setCounter(0);
                            _bemfFlag = true;
                            _adcFlag = false;
                            uint16_t counter = _measureTimer.getCounter();
                            _measureTimer.setCounter(0);
                            _stepCommutationTimer.setPeriod(counter);
                            _stepCommutationTimer.start();
                            if(_bemfPhase == Bldc::MotorPhase::A) {
                                asm("nop");
                            }
                        }
                    }
                    else if(_bemfEdge == Bldc::BemfEdge::Rising) {
                        if(_adcValue > halfSupplyAdcVal) {
                            _measureTimer.stop();
                            _stepCommutationTimer.stop();
                            _stepCommutationTimer.setCounter(0);
                            _bemfFlag = true;
                            _adcFlag = false;
                            uint16_t counter = _measureTimer.getCounter();
                            _stepCommutationTimer.setPeriod(counter);
                            _measureTimer.setCounter(0);
                            _stepCommutationTimer.start();
                            if(_bemfPhase == Bldc::MotorPhase::A) {
                                asm("nop");
                            }
                        }
                    }
                }
            }
        }};

        void openLoopStart() {
            _openLoopStart.give();
            //_openLoop = false;
            //_adc.enable();
        }

        Threading::BinarySemaphore _tim3Event;
        Threading::BinarySemaphore _openLoopStart;
        Threading::BinarySemaphore _onCommTimerUpdate;

        void getInstance();
        bool _adcFlag = false;
        bool _bemfFlag = false;
        bool _openLoop = false;
        static constexpr uint32_t _openLoopEndPeriodUs = 2000;
        static constexpr uint32_t _openLoopStartPeriodUs = 50000;
        int _currentPeriodUs = 1100;

        Drv8328 _drv;
        Adc::AdcCh32vThreePhaseInj& _adc;
        uint16_t _adcValue;

        uint16_t _adcValueA;
        uint16_t _adcValueB;
        uint16_t _adcValueC;

        Timer::TimerBaseCh32v _measureTimer = {timMeas, 0xFFFF, 72};
        Timer::TimerBaseCh32v _stepCommutationTimer = {timComm, 0xFFFF, 72};

        Gpio::Ch32vPin& _testGpio;
    };

}