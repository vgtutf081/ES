#pragma once

#include "ch32v20x_tim.h"
#include "ch32v20x_rcc.h"
#include "ITimer.h"

#include <limits>
#include <cstdint>

namespace ES::Driver::Timer { 
    class PwmCh32v : public IPwm {
    public:
        PwmCh32v(TIM_TypeDef* tim, GPIO_TypeDef* port, u16 pwmPin, u16 channel, u16 div = TIM_CKD_DIV1, u16 arr = 100) : _tim(tim), _port(port), _pwmPin(pwmPin), _channel(channel), _div(div), _arr(arr) {

            uint32_t rccPeriph  = 0;
            if(_port == GPIOA) {
                rccPeriph = RCC_APB2Periph_GPIOA;
            }
            else if(_port == GPIOB) {
                rccPeriph = RCC_APB2Periph_GPIOB;
            }
            else if(_port == GPIOC) {
                rccPeriph = RCC_APB2Periph_GPIOC;
            }
            else if(_port == GPIOD) {
                rccPeriph = RCC_APB2Periph_GPIOD;
            }
            else if(_port == GPIOE) {
                rccPeriph = RCC_APB2Periph_GPIOE;
            }

            uint32_t timPeriph  = 0;
            if(tim == TIM1) {
                timPeriph = RCC_APB2Periph_TIM1;
            }
            else if(tim == TIM2) {
                timPeriph = RCC_APB1Periph_TIM2;
            }
            else if(tim == TIM3) {
                timPeriph = RCC_APB1Periph_TIM3;
            }
            else if(tim == TIM4) {
                timPeriph = RCC_APB1Periph_TIM4;
            }
            else if(tim == TIM5) {
                timPeriph = RCC_APB1Periph_TIM5;
            }

            RCC_APB2PeriphClockCmd(rccPeriph | timPeriph, ENABLE);

            GPIO_InitTypeDef GPIO_InitStructure={0};
            TIM_OCInitTypeDef TIM_OCInitStructure={0};
            TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

            GPIO_InitStructure.GPIO_Pin = pwmPin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(_port, &GPIO_InitStructure);

            TIM_TimeBaseInitStructure.TIM_Period = arr;
            TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;
            TIM_TimeBaseInitStructure.TIM_ClockDivision = div;
            TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
            TIM_TimeBaseInit(_tim, &TIM_TimeBaseInitStructure);

            TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

            TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
            TIM_OCInitStructure.TIM_Pulse = 0;
            TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
            if(channel == 1) {
                TIM_OC1Init(_tim, &TIM_OCInitStructure);
                TIM_OC1PreloadConfig(_tim, TIM_OCPreload_Disable);
            }
            if(channel == 2) {
                TIM_OC2Init(_tim, &TIM_OCInitStructure);
                TIM_OC2PreloadConfig(_tim, TIM_OCPreload_Disable);
            }
            if(channel == 3) {
                TIM_OC3Init(_tim, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(_tim, TIM_OCPreload_Disable);
            }
            if(channel == 4) {
                TIM_OC4Init(_tim, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(_tim, TIM_OCPreload_Disable);
            }
            TIM_CtrlPWMOutputs(_tim, ENABLE);

            TIM_ARRPreloadConfig(_tim, ENABLE);
            TIM_Cmd(_tim, ENABLE);
        }

        ~PwmCh32v() {
            TIM_DeInit(_tim);
        }

        bool setParams(u32 freqHz, float duty) override {
            u32 timFreq = 0;
            RCC_ClocksTypeDef test;
            RCC_GetClocksFreq(&test);
            if(_tim == TIM1 || _tim != nullptr ) {
                timFreq = test.PCLK2_Frequency;
            }
            else {
                timFreq = test.PCLK1_Frequency;
            }
            if(_div == TIM_CKD_DIV2) {
                timFreq /= 2;
            }
            else if(_div == TIM_CKD_DIV4) {
                timFreq /= 4;
            }
            int prescaler = 1;
            prescaler = (timFreq / freqHz) / _arr;
            prescaler -= 1;
            if(prescaler > std::numeric_limits<u16>::max() || prescaler < 0) {
                return false;
            }
            _tim->PSC = prescaler;

            //_tim->CH1CVR = static_cast<uint16_t>(duty * _arr);
            _ccp = static_cast<uint16_t>(duty * _arr);
            return true;
        }

        void start() override {
            if(_channel == 1) {
                _tim->CH1CVR = _ccp;
            }
            else if(_channel == 2) {
                _tim->CH2CVR = _ccp;
            }
            else if(_channel == 3) {
                _tim->CH3CVR = _ccp;
            }
            else if(_channel == 4) {
                _tim->CH4CVR = _ccp;
            }
        }

        void stop() override {
            if(_channel == 1) {
                _tim->CH1CVR = 0;
            }
            else if(_channel == 2) {
                _tim->CH2CVR = 0;
            }
            else if(_channel == 3) {
                _tim->CH3CVR = 0;
            }
            else if(_channel == 4) {
                _tim->CH4CVR = 0;
            }
        }

    private:
        u16 _channel;
        u16 _ccp = 0;
        u16 _arr;
        TIM_TypeDef* _tim;
        GPIO_TypeDef* _port;
        u16 _pwmPin;
        u16 _div;
    };
}