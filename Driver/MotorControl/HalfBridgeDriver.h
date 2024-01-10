#pragma once

#include "GpioCh32v.h"
#include "TimerCh32.h"
#include "Bldc.h"

#if defined CH32V203F8U6
#define PHASEA_HIGH GPIO_Pin_10
#define PHASEB_HIGH GPIO_Pin_9
#define PHASEC_HIGH GPIO_Pin_8
#define PHASEA_LOW GPIO_Pin_1
#define PHASEB_LOW GPIO_Pin_0
#define PHASEC_LOW GPIO_Pin_7

#define PHASEA_HIGH_GPIOPORT GPIOA
#define PHASEB_HIGH_GPIOPORT GPIOA
#define PHASEC_HIGH_GPIOPORT GPIOA
#define PHASEA_LOW_GPIOPORT GPIOB
#define PHASEB_LOW_GPIOPORT GPIOB
#define PHASEC_LOW_GPIOPORT GPIOA
#endif


namespace ES::Driver::MotorControl {

static constexpr uint32_t DeadTime = 45;

class Drv8328 {
    public:
        Drv8328(Gpio::Ch32vPin& nSleep, Gpio::Ch32vPin& nFault, Timer::TimerBaseCh32v& timComplimentary, Timer::PwmCh32v& pwmA, Timer::PwmCh32v& pwmB, Timer::PwmCh32v& pwmC) : _nSleep(nSleep), _nFault(nFault), _timComplimentary(timComplimentary), _pwmA(pwmA), _pwmB(pwmB), _pwmC(pwmC) {
            _timComplimentary.bdtrConfig(DeadTime);
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

        void setCompare(uint16_t value, Bldc::MotorPhase phase) {
            getNodeByPhase(phase).setCompare(value);
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
            deCommutate(Bldc::MotorPhase::A);
            deCommutate(Bldc::MotorPhase::B);
            deCommutate(Bldc::MotorPhase::C);
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

        float _duty = 0.f;

        Timer::PwmCh32v& _pwmA;
        Timer::PwmCh32v& _pwmB;
        Timer::PwmCh32v& _pwmC;

        Timer::PwmCh32v _currentNode;

        Gpio::Ch32vPin& _nFault;
        Gpio::Ch32vPin& _nSleep;
    };

    class FD6288 {
    public:
        FD6288(uint16_t arr) {
            _pwmA.addComplimetary(PHASEA_LOW_GPIOPORT, PHASEC_LOW);
            _pwmB.addComplimetary(PHASEB_LOW_GPIOPORT, PHASEB_LOW);
            _pwmC.addComplimetary(PHASEC_LOW_GPIOPORT, PHASEA_LOW);
            _timComplimentary.configCC4();
            _timComplimentary.bdtrConfig(DeadTime);

            _timComplimentary.setPeriod(arr);

            _pwmA.start();
            _pwmA.startComplimentary();
            _pwmB.start();
            _pwmB.startComplimentary();
            _pwmC.start();
            _pwmC.startComplimentary();

            _timComplimentary.start();
            _timComplimentary.enableCtrlPwmOutputs();
            _timComplimentary.generateUpdateEvent();
            //deCommutateAll();
        }

        void startPwm() {
            _timComplimentary.start();
        }
        
        void stopPwm() {
           _timComplimentary.stop();
        }

        void demagnetize(uint16_t duty) {
            setDuty(duty);
            commutateLow(Bldc::MotorPhase::A);
            commutateLow(Bldc::MotorPhase::B);
            commutateLow(Bldc::MotorPhase::C);
        }

        void setDuty(uint16_t duty) {
            _duty = duty;
            _pwmA.setCompare(duty);
            _pwmB.setCompare(duty);
            _pwmC.setCompare(duty);
        }

#if defined CH32V203F8U6
        void commutateLow(Bldc::MotorPhase phase) {
            if(phase == Bldc::MotorPhase::A) {
                PHASEA_LOW_GPIOPORT->CFGLR &= ~(0xf << 4);
                PHASEA_LOW_GPIOPORT->CFGLR |= (0x3 << 4);
                PHASEA_LOW_GPIOPORT->BSHR = PHASEA_LOW;

                PHASEA_HIGH_GPIOPORT->CFGHR &= ~(0xf << 8);
                PHASEA_HIGH_GPIOPORT->CFGHR |= (0x3 << 8);
                PHASEA_HIGH_GPIOPORT->BCR = PHASEA_HIGH;
            }
            if(phase == Bldc::MotorPhase::B) {
                PHASEB_LOW_GPIOPORT->CFGLR &= ~(0xf << 0); 
                PHASEB_LOW_GPIOPORT->CFGLR|= (0x3 << 0);
                PHASEB_LOW_GPIOPORT->BSHR = PHASEB_LOW;

                PHASEB_HIGH_GPIOPORT->CFGHR &= ~(0xf << 4); 
                PHASEB_HIGH_GPIOPORT->CFGHR |= (0x3 << 4);
                PHASEB_HIGH_GPIOPORT->BCR = PHASEB_HIGH;
            }
            if(phase == Bldc::MotorPhase::C) {
                PHASEC_LOW_GPIOPORT->CFGLR &= ~(0xf << 28);
                PHASEC_LOW_GPIOPORT->CFGLR |= (0x3 << 28);
                PHASEC_LOW_GPIOPORT->BSHR = PHASEC_LOW;

                PHASEC_HIGH_GPIOPORT->CFGHR &= ~(0xf << 0);
                PHASEC_HIGH_GPIOPORT->CFGHR |= (0x3 << 0);
                PHASEC_HIGH_GPIOPORT->BCR = PHASEC_HIGH;
            }
        }

        void commutateHigh(Bldc::MotorPhase phase) {
            if(phase == Bldc::MotorPhase::A) {
			    PHASEA_LOW_GPIOPORT->CFGLR &= ~(0xf << 4);
                PHASEA_LOW_GPIOPORT->CFGLR|= (0xb << 4);
		        PHASEA_HIGH_GPIOPORT->CFGHR &= ~(0xf << 8);
                PHASEA_HIGH_GPIOPORT->CFGHR |= (0xb << 8);
            }
            if(phase == Bldc::MotorPhase::B) {
                PHASEB_LOW_GPIOPORT->CFGLR &= ~(0xf << 0);
                PHASEB_LOW_GPIOPORT->CFGLR|= (0xb << 0);
                PHASEB_HIGH_GPIOPORT->CFGHR &= ~(0xf << 4);
                PHASEB_HIGH_GPIOPORT->CFGHR |= (0xb << 4);
            }
            if(phase == Bldc::MotorPhase::C) {
                PHASEC_LOW_GPIOPORT->CFGLR &= ~(0xf << 28);
                PHASEC_LOW_GPIOPORT->CFGLR |= (0xb << 28);
                PHASEC_HIGH_GPIOPORT->CFGHR &= ~(0xf << 0);
                PHASEC_HIGH_GPIOPORT->CFGHR |= (0xb << 0);
            }
        }

        void deCommutate(Bldc::MotorPhase phase) {
            if(phase == Bldc::MotorPhase::A) {
                PHASEA_LOW_GPIOPORT->CFGLR &= ~(0xf <<  4);
                PHASEA_LOW_GPIOPORT->CFGLR|= (0x3 << 4);
                PHASEA_LOW_GPIOPORT->BCR = PHASEA_LOW;

                PHASEA_HIGH_GPIOPORT->CFGHR &= ~(0xf << 8);
                PHASEA_HIGH_GPIOPORT->CFGHR |= (0x3 << 8);
                PHASEA_HIGH_GPIOPORT->BCR = PHASEA_HIGH;
            }
            if(phase == Bldc::MotorPhase::B) {
                PHASEB_LOW_GPIOPORT->CFGLR &= ~(0xf << 0);
                PHASEB_LOW_GPIOPORT->CFGLR|= (0x3 << 0);
                PHASEB_LOW_GPIOPORT->BCR = PHASEB_LOW;

                PHASEB_HIGH_GPIOPORT->CFGHR &= ~(0xf << 4);
                PHASEB_HIGH_GPIOPORT->CFGHR |= (0x3 << 4);
                PHASEB_HIGH_GPIOPORT->BCR = PHASEB_HIGH;
            }
            if(phase == Bldc::MotorPhase::C) {
                PHASEC_LOW_GPIOPORT->CFGLR &= ~(0xf << 28);
                PHASEC_LOW_GPIOPORT->CFGLR |= (0x3 << 28);
                PHASEC_LOW_GPIOPORT->BCR = PHASEC_LOW;

                PHASEC_HIGH_GPIOPORT->CFGHR &= ~(0xf << 0);
                PHASEC_HIGH_GPIOPORT->CFGHR |= (0x3 << 0);
                PHASEC_HIGH_GPIOPORT->BCR = PHASEC_HIGH;
            }
        }

#else

        void setCompare(uint16_t value, Bldc::MotorPhase phase) {
            getNodeByPhase(phase).setCompare(value);
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
                return _pwmB;
            }
            else {
                return _pwmC;
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
#endif
        constexpr uint16_t getDeadTime() const {
            return DeadTime;
        }

        constexpr uint16_t getDuty() const {
            return _duty;
        }

        uint16_t getPeriod() {
            return _timComplimentary.getPeriod();
        }

        void setPeriod(uint16_t period) {
            return _timComplimentary.setPeriod(period);
        }   

        void setPrescaler(uint16_t prescaler) {
            return _timComplimentary.setPrescaler(prescaler);
        }

        uint32_t getTimeBaseFreq() {
            return _timComplimentary.getTimBaseFreq();
        }

    private:
    
        Timer::TimerBaseCh32v _timComplimentary{TIM1, 0xFFFF};
        Timer::PwmCh32v _pwmC{_timComplimentary, PHASEC_HIGH_GPIOPORT, PHASEC_HIGH, 1};
        Timer::PwmCh32v _pwmB{_timComplimentary, PHASEB_HIGH_GPIOPORT, PHASEB_HIGH, 2};
        Timer::PwmCh32v _pwmA{_timComplimentary, PHASEA_HIGH_GPIOPORT, PHASEA_HIGH, 3};
       
        uint16_t _duty = 0;

        Timer::PwmCh32v _currentNode;
    };
}
