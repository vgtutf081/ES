#pragma once

#include "IGpio.h"
#include "TimerCh32.h"
#include "Semaphore.h"
#include "Bldc.h"

namespace ES::Driver::MotorControl {

    class Drv8328 {
    public:
        Drv8328(Gpio::IGpio& nSleep, Gpio::IGpio& nFault, Timer::TimerBaseCh32v& timComplimentary, Timer::PwmCh32v& pwmA, Timer::PwmCh32v& pwmB, Timer::PwmCh32v& pwmC) : _nSleep(nSleep), _nFault(nFault), _timComplimentary(timComplimentary), _pwmA(pwmA), _pwmB(pwmB), _pwmC(pwmC) {

            _timComplimentary.bdtrConfig();
            _timComplimentary.setFreq(_freq);
            _pwmA.setParams(0.3f);
            _pwmB.setParams(0.3f);
            _pwmC.setParams(0.3f);
            _nSleep.configureOutput();
            _nSleep.reset();
            _nFault.configureInput(Gpio::PullMode::Up);
            deCommutateAll();
        }

        void init() {
             _nSleep.set();
        }

        void deInit() {
             _nSleep.reset();
        }

        void startPwm() {
            _timComplimentary.start();
        }
        
        void stopPwm() {
           _timComplimentary.stop();
        }

        void setDuty(float duty) {
            _pwmA.setParams(duty);
            _pwmB.setParams(duty);
            _pwmC.setParams(duty);
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

        Timer::PwmCh32v& getNodeByPhase(Bldc::MotorPhase phase) {
            if(phase == Bldc::MotorPhase::A) {
                return _pwmA;
            }
            else if(phase == Bldc::MotorPhase::B) {
                return  _pwmB;
            }
            else {
                return  _pwmC;
            }
        }

        void deCommutateAll() {
            deCommutate(_pwmA);
            deCommutate(_pwmB);
            deCommutate(_pwmC);
        }

        void commutateLow(Bldc::MotorPhase phase) {
            commutateLow(getNodeByPhase(phase));
        }

        void commutateHigh(Bldc::MotorPhase phase) {
           commutateHigh(getNodeByPhase(phase));
        }

        void deCommutate(Bldc::MotorPhase phase) {
           deCommutate(getNodeByPhase(phase));
        }

        bool adcFlag = false;

        Timer::TimerBaseCh32v& _timComplimentary;

    private:

        Timer::PwmCh32v& _pwmA;
        Timer::PwmCh32v& _pwmB;
        Timer::PwmCh32v& _pwmC;

        Timer::PwmCh32v _currentNode;

        Gpio::IGpio& _nFault;
        
        int _freq = 32000;
        Gpio::IGpio& _nSleep;

    };
}