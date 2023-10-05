#pragma once

#include "IGpio.h"
#include "ITimer.h"
#include "AdcCh32v.h"
#include "BinarySemaphore.h"

namespace ES::Driver::MotorControl {

    enum BldcStep {
        ChAl,
        ChBl,
        AhBl,
        AhCl,
        BhCl,
        BhAl
    };

    enum MotorPhase {
        A,
        B,
        C
    };

    enum BemfEdge {
        Rising,
        Falling
    };

    class Drv8328 {
    public:
        Drv8328(Gpio::IGpio& nSleep, Gpio::IGpio& nFault, Timer::PwmCh32v& pwmA, Timer::PwmCh32v& pwmB, Timer::PwmCh32v& pwmC, Adc::AdcCh32vSingleEnded& adcA, Adc::AdcCh32vSingleEnded& adcB, Adc::AdcCh32vSingleEnded& adcC) : _nSleep(nSleep), _nFault(nFault), _pwmA(pwmA), _pwmB(pwmB), _pwmC(pwmC), _adcA(adcA), _adcB(adcB), _adcC(adcC) {
            _pwmA.setParams(_freq, 0.2f);
            _pwmB.setParams(_freq, 0.2f);
            _pwmC.setParams(_freq, 0.2f);
            _nSleep.configureOutput();
            _nSleep.reset();
            _nFault.configureInput(Gpio::PullMode::Up);
        }

        void init() {
             _nSleep.set();
        }

        void deInit() {
             _nSleep.reset();
        }

        void startPwm() {
            _pwmA.enable();
        }
        
        void stopPwm() {
           //_pwmA.disable();
        }

        void setDuty(float duty) {
            _pwmA.setParams(_freq, duty);
            _pwmB.setParams(_freq, duty);
            _pwmC.setParams(_freq, duty);
        }

        void commutateLow(Timer::PwmCh32v& node) {
            node.stop();
            node.startComplimentary();
        }

        void commutateHigh(Timer::PwmCh32v& node) {
            node.start();
            node.stopComplimentary();
        }

        void deCommutate(Timer::PwmCh32v& node) {
            node.stop();
            node.stopComplimentary();
        }

        void nextStep() {
            steps++;
            if(_nextStep == BldcStep::ChAl) {
                commutateHigh(_pwmC);
                commutateLow(_pwmA);
                deCommutate(_pwmB);
                _nextStep = BldcStep::ChBl;
                _bemfPhase = MotorPhase::B;
                _bemfEdge = BemfEdge::Falling;
            }
            else if(_nextStep == BldcStep::ChBl) {
                commutateHigh(_pwmC);
                commutateLow(_pwmB);
                deCommutate(_pwmA);
                _nextStep = BldcStep::AhBl;
                _bemfPhase = MotorPhase::A;
                _bemfEdge = BemfEdge::Rising;
            }
            else if(_nextStep == BldcStep::AhBl) {
                commutateHigh(_pwmA);
                commutateLow(_pwmB);
                deCommutate(_pwmC);
                _nextStep = BldcStep::AhCl;
                _bemfPhase = MotorPhase::C;
                _bemfEdge = BemfEdge::Falling;
            }
            else if(_nextStep == BldcStep::AhCl) {
                commutateHigh(_pwmA);
                commutateLow(_pwmC);
                deCommutate(_pwmB);
                _nextStep = BldcStep::BhCl;
                _bemfPhase = MotorPhase::B;
                _bemfEdge = BemfEdge::Rising;
            }
            else if(_nextStep == BldcStep::BhCl) {
                commutateHigh(_pwmB);
                commutateLow(_pwmC);
                deCommutate(_pwmA);
                _nextStep = BldcStep::BhAl;
                _bemfPhase = MotorPhase::A;
                _bemfEdge = BemfEdge::Falling;
            }
            else if(_nextStep == BldcStep::BhAl) {
                commutateHigh(_pwmB);
                commutateLow(_pwmA);
                deCommutate(_pwmC);
                _nextStep = BldcStep::ChAl;
                _bemfPhase = MotorPhase::C;
                _bemfEdge = BemfEdge::Rising;
            }
            adcFlag = true;
            //count = 0;
        }

        uint32_t steps = 0;
        uint32_t count = 0;

        MotorPhase _bemfPhaseStart;
        BemfEdge _bemfEdgeStart;

        MotorPhase _bemfPhaseEnd;
        BemfEdge _bemfEdgeEnd;
        

        bool openLoop = true;

        Timer::PwmCh32v& _pwmA;
        Timer::PwmCh32v& _pwmB;
        Timer::PwmCh32v& _pwmC;

        Adc::AdcCh32vSingleEnded& _adcA;
        Adc::AdcCh32vSingleEnded& _adcB;
        Adc::AdcCh32vSingleEnded& _adcC;

        Adc::AdcCh32vSingleEnded& _adcBemf = _adcA;

        BldcStep _nextStep = BldcStep::ChAl;
        MotorPhase _bemfPhase = MotorPhase::B;
        BemfEdge _bemfEdge;

        uint16_t _halfSupplyVoltage = 5000;

        bool adcFlag = false;
        bool bemfFlag = false;

        Gpio::IGpio& _nFault;

        Threading::BinarySemaphore adcSem;

        bool _adcSem = false;

    private:

        int _freq = 32000;
        Gpio::IGpio& _nSleep;

    };
}