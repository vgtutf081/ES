#pragma once

#include "IGpio.h"
#include "TimerCh32.h"
#include "Bldc.h"

namespace ES::Driver::MotorControl {

    class Drv8328 {
    public:
        Drv8328(Gpio::IGpio& nSleep, Gpio::IGpio& nFault, Timer::TimerBaseCh32v& timComplimentary, Timer::PwmCh32v& pwmA, Timer::PwmCh32v& pwmB, Timer::PwmCh32v& pwmC) : _nSleep(nSleep), _nFault(nFault), _timComplimentary(timComplimentary), _pwmA(pwmA), _pwmB(pwmB), _pwmC(pwmC) {

            _timComplimentary.bdtrConfig();
            //_timComplimentary.setPeriod(3000);
            //_pwmA.setParams(_duty);
            //_pwmB.setParams(_duty);
            //_pwmC.setParams(_duty);
            _nSleep.configureOutput();
            _nSleep.reset();
            _nFault.configureInput(Gpio::PullMode::Up);
            deCommutateAll();
        }

        void setTorque(uint16_t value) {
            _timComplimentary.setPeriod(value);
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
            _duty = duty;
            _pwmA.setParams(duty);
            _pwmB.setParams(duty);
            _pwmC.setParams(duty);
        }

        void setCompare(uint16_t value) {
            _pwmA.setCompare(value);
            _pwmB.setCompare(value);
            _pwmC.setCompare(value);
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

        float getDuty() {
            return _duty;
        }

        uint16_t getPeriod() {
            return _timComplimentary.getPeriod();
        }

        Timer::TimerBaseCh32v& _timComplimentary;

    private:
        
        static constexpr float _miniamalDutyChange = 0.001f;
        float _duty = 0.27f;

        Timer::PwmCh32v& _pwmA;
        Timer::PwmCh32v& _pwmB;
        Timer::PwmCh32v& _pwmC;

        Timer::PwmCh32v _currentNode;

        Gpio::IGpio& _nFault;
        
        int _freq = 32000;
        Gpio::IGpio& _nSleep;

    };
}