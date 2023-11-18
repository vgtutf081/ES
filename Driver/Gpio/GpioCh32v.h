#pragma once

#include "ch32v20x_gpio.h"
#include "ch32v20x.h"
#include "ch32v20x_rcc.h"
#include "GpioCh32Modes.h"
#include "IGpio.h"

namespace ES::Driver::Gpio {

    class Ch32vPin {
        public:
        Ch32vPin(GPIO_TypeDef* port, uint16_t pin) : _port(port), _pin(pin) {
            uint32_t rccPeriph  = 0;

            if(port == GPIOA){
                rccPeriph = RCC_APB2Periph_GPIOA;
            }
            else if(port == GPIOB){
                rccPeriph = RCC_APB2Periph_GPIOB;
            }
            else if(port == GPIOC){
                rccPeriph = RCC_APB2Periph_GPIOC;
            }
            else if(port == GPIOD){
                rccPeriph = RCC_APB2Periph_GPIOD;
            }
            else if(port == GPIOE){
                rccPeriph = RCC_APB2Periph_GPIOE;
            }

            RCC_APB2PeriphClockCmd(rccPeriph,ENABLE);
        }

        uint32_t getPin() const   {
            return _pin;
        }

        uint32_t getPort() const  {
            return reinterpret_cast<uint32_t>(_port);
            
        }

        void set()  {
            GPIO_SetBits(_port, _pin);
            _pinSet = true;
        }

        void reset()  {
            GPIO_ResetBits(_port, _pin);
            _pinSet = false;
        }

        void toggle()  {
            if(_pinSet) {
                reset();
            }
            else {
                set();
            }
        }

        uint32_t getPortAndPin() const  {
            return 0;
        }

        bool read() const  {
            return GPIO_ReadInputDataBit(_port, _pin);
        }

        void disable()  {
            GPIO_InitTypeDef  GPIO_InitStructure={0};
            GPIO_InitStructure.GPIO_Pin = _pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(_port, &GPIO_InitStructure);
        }

        inline void configureOutput(DriveMode driveMode = DriveMode::PushPull, PullMode pullMode = PullMode::None) {
            setMode(PinMode::Output, driveMode, pullMode);
        }

        inline void configureInput(PullMode pullMode = PullMode::None) {
            setMode(PinMode::Input, DriveMode::None, pullMode);
        }

        void setMode(PinMode mode, DriveMode drive, PullMode pull) {
            GPIOMode_TypeDef modeDrivePull;
            if(mode == PinMode::AnalogInput) {
                modeDrivePull = GPIO_Mode_AIN;
            }
            else if(mode == PinMode::Input) {
                if(pull == PullMode::Up) {
                    modeDrivePull = GPIO_Mode_IPU;
                }
                else if(pull == PullMode::Down) {
                    modeDrivePull = GPIO_Mode_IPD;
                }
                else {
                    modeDrivePull = GPIO_Mode_IN_FLOATING;
                }
            }
            else if(mode == PinMode::Alternate) {
                if(drive == DriveMode::PushPull) {
                    modeDrivePull = GPIO_Mode_AF_PP;
                }
                else if(drive == DriveMode::OpenDrain) {
                    modeDrivePull = GPIO_Mode_AF_OD;
                }
            }
            else if(mode == PinMode::Output) {
                if(drive == DriveMode::PushPull) {
                    modeDrivePull = GPIO_Mode_Out_PP;
                }
                else if(drive == DriveMode::OpenDrain) {
                    modeDrivePull = GPIO_Mode_Out_OD;
                }
            }

            GPIO_InitTypeDef  GPIO_InitStructure={0};
            GPIO_InitStructure.GPIO_Pin = _pin;
            GPIO_InitStructure.GPIO_Mode = modeDrivePull;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(_port, &GPIO_InitStructure);
        }
        private:
        bool _pinSet = false;
        GPIO_TypeDef* _port;
        uint16_t _pin;
    };
}
