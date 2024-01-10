#pragma once

#include "HalfBridgeDriver.h"
#include "TimerCh32.h"
#include "GpioCh32v.h"
#include "Dshot.h"
#include "Comparator.h"

#include "delayCustom.h"

#include <cmath>

#define BR2205

namespace ES::Driver::MotorControl {

    static constexpr float MinThrottle = 0.03f;
    static constexpr float MaxThrottle = 1.f;
    static constexpr float MinStartupThrottle = 0.03f;
    static constexpr uint16_t DefaultPrescaler = 1;

    enum Tune {
        Startup,
        Input
    };

    class Esc6Step {
    public:
        enum MotorState {
            Idle,
            OpenLoop,
            Started,
            Failed,
            PreStart
        };

        enum class AngleAdvantage : uint16_t {
            Degree0 = 1,
            Degree15 = 2,
            Degree30 = 0xFFFF
        };

        Esc6Step(uint16_t period) : _period(period) { 
            _minimumDuty = (period * MinThrottle);
            _maximumDuty = (period * MaxThrottle);
            _minStartupDuty = (period * MinStartupThrottle);
            getInstance();
            //_drv.startPwm();
        }

        constexpr uint16_t getMinDuty() const {
            return _minimumDuty;
        }

        constexpr uint16_t getMinStartupDuty() const {
            return _minStartupDuty;
        }

        constexpr uint16_t getMaxDuty() const {
            return _maximumDuty;
        }

        constexpr uint16_t getDuty() const {
            return _actualDuty;
        }

        void setFrequency(uint32_t value) {
            uint32_t freq = _drv.getTimeBaseFreq();
            _drv.setPeriod(freq / value);
            asm("nop");
        }

        void setDuty(uint16_t value) {
            _drv.setDuty(value);
        }

        void dropPhases() {
            _drv.deCommutate(Bldc::MotorPhase::A);
            _drv.deCommutate(Bldc::MotorPhase::B);
            _drv.deCommutate(Bldc::MotorPhase::C);
        }

        void prepareStartup() {
            setDuty(_minStartupDuty);
        }

        constexpr uint16_t getDeadTime() const {
            return _drv.getDeadTime();
        }

        void playTune(Tune tune) {
            if(tune == Tune::Startup) {
                __disable_irq();
                _drv.setPeriod(2000);
                //IWDG_ReloadCounter();
                setDuty(_volume);
                switchPhase(Bldc::Step::ChAl);
                _drv.setPrescaler(80);
                Delay::delayMs(200);
                switchPhase(Bldc::Step::BhCl);
                _drv.setPrescaler(60);
                Delay::delayMs(200);
                switchPhase(Bldc::Step::AhCl);
                _drv.setPrescaler(20);
                Delay::delayMs(200);
                dropPhases();
                _drv.setPrescaler(DefaultPrescaler);
                __enable_irq();

            }

            if(tune == Tune::Input) {
                __disable_irq();
                _drv.setPeriod(2000);
                //IWDG_ReloadCounter();
                Delay::delayMs(100);
                switchPhase(Bldc::Step::ChAl);
                _drv.setPrescaler(40);
                setDuty(_volume);
                Delay::delayMs(100);
                _drv.setPrescaler(30);
                Delay::delayMs(100);
                _drv.setPrescaler(20);
                Delay::delayMs(100);
                dropPhases();
                _drv.setPrescaler(DefaultPrescaler);
                __enable_irq();
            }

        }

        void switchPhase(Bldc::Step step) {
            _currentStep = step;
            if(step == Bldc::Step::ChAl) {
                _drv.commutateLow(Bldc::MotorPhase::A);
                _drv.deCommutate(Bldc::MotorPhase::B);
                _activeHighPhase = Bldc::MotorPhase::C;
                _previousStep = Bldc::Step::BhAl;
                _nextStep = Bldc::Step::ChBl;
                _bemfPhase = Bldc::MotorPhase::B;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::ChBl) {
                _drv.commutateLow(Bldc::MotorPhase::B);
                _drv.deCommutate(Bldc::MotorPhase::A);
                _activeHighPhase = Bldc::MotorPhase::C;
                _previousStep = Bldc::Step::ChAl;
                _nextStep = Bldc::Step::AhBl;
                _bemfPhase = Bldc::MotorPhase::A;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            else if(step == Bldc::Step::AhBl) {
                _drv.commutateLow(Bldc::MotorPhase::B);
                _drv.deCommutate(Bldc::MotorPhase::C);
                _activeHighPhase = Bldc::MotorPhase::A;
                _previousStep = Bldc::Step::ChBl;
                _nextStep = Bldc::Step::AhCl;
                _bemfPhase = Bldc::MotorPhase::C;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::AhCl) {
                _drv.commutateLow(Bldc::MotorPhase::C);
                _drv.deCommutate(Bldc::MotorPhase::B);
                _activeHighPhase = Bldc::MotorPhase::A;
                _previousStep = Bldc::Step::AhBl;
                _nextStep = Bldc::Step::BhCl;
                _bemfPhase = Bldc::MotorPhase::B;
                _bemfEdge = Bldc::BemfEdge::Rising;
            }
            else if(step == Bldc::Step::BhCl) {
                _drv.commutateLow(Bldc::MotorPhase::C);
                _drv.deCommutate(Bldc::MotorPhase::A);
                _activeHighPhase = Bldc::MotorPhase::B;
                _previousStep = Bldc::Step::AhCl;
                _nextStep = Bldc::Step::BhAl;
                _bemfPhase = Bldc::MotorPhase::A;
                _bemfEdge = Bldc::BemfEdge::Falling;
            }
            else if(step == Bldc::Step::BhAl) {
                _drv.commutateLow(Bldc::MotorPhase::A);
                _drv.deCommutate(Bldc::MotorPhase::C);
                _activeHighPhase = Bldc::MotorPhase::B;
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
            //_drv.setDuty(_actualDuty);
            _drv.commutateHigh(_activeHighPhase);
        }

        void nextStep() {
            if(_forwardDirection) {
                switchPhase(_nextStep);
            }
            else {
                switchPhase(_previousStep);
            }
        }

        void reconfigureComparator() {
            if((_currentStep == Bldc::Step::AhBl) || (_currentStep == Bldc::Step::BhAl)) {
                _bemfComps.setOutput(Comparator::CompInstance::Opa1Out0);
            }           
            if((_currentStep == Bldc::Step::ChBl) || (_currentStep == Bldc::Step::BhCl)) {
                _bemfComps.setOutput(Comparator::CompInstance::Opa2In1Out0);
            }      
            if((_currentStep == Bldc::Step::ChAl) || (_currentStep == Bldc::Step::AhCl)) {
                _bemfComps.setOutput(Comparator::CompInstance::Opa2In0Out0);
            }
        }

        void enableCompInterrupts() {
            _bemfComps.enableInt();
        }

        void maskCompInterrupts() {
            _bemfComps.maskInt();
        }

        uint8_t getCurrentStep() {
            return static_cast<uint8_t>(_currentStep);
        }
        
        void intTim1CC4Callback() {
        }

        void intTim1Callback() {
        }

        void reverseDirection() {
            _forwardDirection = !_forwardDirection;
        }

        MotorState getState() {
            return _state;
        }

        bool getBemfState() {
            if(_bemfEdge == Bldc::BemfEdge::Rising) {
                return !_bemfComps.getOutputState();
            }
            else if(_bemfEdge == Bldc::BemfEdge::Falling) {
                return _bemfComps.getOutputState();        
            }
            else {
                return false;
            }
        }

        bool edgeIsRising() {
            return _bemfEdge == Bldc::BemfEdge::Rising;
        }

    private:

        Comparator::BemfComparators _bemfComps{};

        Driver::MotorControl::FD6288 _drv{_period}; 

        uint8_t _volume = 25;


        uint16_t _period;
        uint16_t _minimumDuty;
        uint16_t _maximumDuty;
        uint16_t _minStartupDuty;

        MotorState _state = MotorState::Idle;

        uint16_t _actualDuty = 0;

        volatile Bldc::Step _currentStep = static_cast<Bldc::Step>(1);
        volatile Bldc::Step _previousStep = static_cast<Bldc::Step>(6);
        volatile Bldc::Step _nextStep = static_cast<Bldc::Step>(2);
        volatile Bldc::MotorPhase _bemfPhase;
        Bldc::BemfEdge _bemfEdge;
        Bldc::MotorPhase _activeHighPhase;

        bool _forwardDirection = false;

        void getInstance();
        
    };

}
