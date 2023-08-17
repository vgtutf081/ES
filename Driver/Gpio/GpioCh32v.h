#pragma once

#include "ch32v20x_gpio.h"
#include "ch32v20x.h"
#include "ch32v20x_rcc.h"

namespace ES::Driver::Gpio {

    struct Ch32vPin {
        public:
        constexpr Ch32vPin(GPIO_TypeDef* port, uint16_t pin, GPIOMode_TypeDef mode = GPIO_Mode_Out_PP) : _port(port), _pin(pin) {
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
            GPIO_InitTypeDef  GPIO_InitStructure={0};
            GPIO_InitStructure.GPIO_Pin = pin;
            GPIO_InitStructure.GPIO_Mode = mode;
            GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
            GPIO_Init(_port, &GPIO_InitStructure);
        }

        constexpr uint16_t getPin() const {
            return _pin;
        }

        constexpr GPIO_TypeDef* getPort() const {
            return _port;
        }

        void set() {
            GPIO_SetBits(_port, _pin);
        }

        void reset() {
            GPIO_ResetBits(_port, _pin);
        }

        private:
        GPIO_TypeDef* _port;
        uint16_t _pin;
    };
}