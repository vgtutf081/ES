#pragma once

#include "IGpio.h"
#include "ITimer.h"

namespace ES::Driver::MotorControl {

    class Drv8328 {
    public:
        Drv8328(Gpio::IGpio& nSleep, Gpio::IGpio& nFault, Gpio::IGpio& LA, Gpio::IGpio& LB, Gpio::IGpio& LC, Timer::IPwm& pwmHA, Timer::IPwm& pwmHB, Timer::IPwm& pwmHC) : _nSleep(nSleep), _nFault(nFault), _LA(LA), _LB(LB), _LC(LC), _pwmHA(pwmHA), _pwmHB(pwmHB), _pwmHC(pwmHC) {
            _LA.configureOutput();
            _LB.configureOutput();
            _LC.configureOutput();
            _pwmHA.setParams(3000, 0.025f);
            _pwmHB.setParams(3000, 0.025f);
            _pwmHC.setParams(3000, 0.025f);
            _LA.reset();
            _LB.reset();
            _LC.reset();
            _nSleep.configureOutput();
            _nSleep.reset();
            _nFault.configureInput(Gpio::PullMode::Up);


        }

        void init() {
            _nSleep.set();
        }

        void commutate(Timer::IPwm& highNode, Gpio::IGpio& lowNode) {
            highNode.start();
            lowNode.set();
        }

        void deCommutate(Timer::IPwm& highNode, Gpio::IGpio& lowNode) {
            highNode.stop();
            lowNode.reset();
        }

        Gpio::IGpio& _LA;
        Gpio::IGpio& _LB;
        Gpio::IGpio& _LC;
        Timer::IPwm& _pwmHA;
        Timer::IPwm& _pwmHB;
        Timer::IPwm& _pwmHC;

    private:

        Gpio::IGpio& _nSleep;
        Gpio::IGpio& _nFault;

    };
}