#pragma once

#include "ch32v20x_tim.h"
#include "ch32v20x_rcc.h"
#include "ITimer.h"

#include <limits>
#include <cstdint>

namespace ES::Driver::Timer { 

    const uint16_t TimerMax = 0xFFFF;

    class PwmCh32v : public IPwm {
    public:
        PwmCh32v(TIM_TypeDef* tim, GPIO_TypeDef* port, u16 pwmPin, u16 channel, u16 div = TIM_CKD_DIV1, u16 arr = 500) : _tim(tim), _port(port), _pwmPin(pwmPin), _channel(channel), _div(div), _arr(arr) {

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
            TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
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
            TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);

            /*TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
            TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
            TIM_OCInitStructure.TIM_Pulse = 20;
            TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
            TIM_OC4Init(TIM1, &TIM_OCInitStructure);*/

            TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC4Ref);

            TIM_Cmd(_tim, DISABLE);
        }

        void addComplimetary(GPIO_TypeDef* portN, u16 pwmPinN) {
            _portN = portN;
            _pwmPinN = pwmPinN;

            uint32_t rccPeriph = 0;
            if(_portN != _port) {
                if(_portN == GPIOB) {
                    rccPeriph |= RCC_APB2Periph_GPIOB;
                }
                else if(_portN == GPIOC) {
                    rccPeriph |= RCC_APB2Periph_GPIOC;
                }
                else if(_portN == GPIOD) {
                    rccPeriph |= RCC_APB2Periph_GPIOD;
                }
                else if(_portN == GPIOE) {
                    rccPeriph |= RCC_APB2Periph_GPIOE;
                }
            }
            RCC_APB2PeriphClockCmd(rccPeriph, ENABLE);

            GPIO_InitTypeDef GPIO_InitStructure={0};
            GPIO_InitStructure.GPIO_Pin = _pwmPinN;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(_portN, &GPIO_InitStructure);

            TIM_BDTRInitTypeDef     TIM_BDTRInitStructure = {0};
            TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;
            TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;
            TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
            TIM_BDTRInitStructure.TIM_DeadTime = 0x0;
            TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
            TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
            TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
            TIM_BDTRConfig(_tim, &TIM_BDTRInitStructure);

            TIM_OCInitTypeDef TIM_OCInitStructure={0};
            TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

            TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
            TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
            TIM_OCInitStructure.TIM_Pulse = 0;
            TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
            TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
            TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
            TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
            if(_channel == 1) {
                TIM_OC1Init(_tim, &TIM_OCInitStructure);
                TIM_OC1PreloadConfig(_tim, TIM_OCPreload_Disable);
                //_tim->CCER |= TIM_CC1NP;
                //_tim->CCER |= TIM_CC1P;
            }
            else if(_channel == 2) {
                TIM_OC2Init(_tim, &TIM_OCInitStructure);
                TIM_OC2PreloadConfig(_tim, TIM_OCPreload_Disable);
                //_tim->CCER |= TIM_CC2NP;
                //_tim->CCER |= TIM_CC2P;
            }
            else if(_channel == 3) {
                TIM_OC3Init(_tim, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(_tim, TIM_OCPreload_Disable);
                //_tim->CCER |= TIM_CC3NP;
                //_tim->CCER |= TIM_CC3P;
            }
            else if(_channel == 4) {
                TIM_OC4Init(_tim, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(_tim, TIM_OCPreload_Disable);
                //_tim->CCER |= TIM_CC4NP;
                //_tim->CCER |= TIM_CC4P;
            }

        }

        ~PwmCh32v() {
            //disable();
            //TIM_DeInit(_tim);

        }

        void setIrq(uint8_t priority, uint16_t event = TIM_IT_CC4) {
                TIM_OCInitTypeDef TIM_OCInitStructure={0};
                TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;

                TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
                TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
                TIM_OCInitStructure.TIM_Pulse = 0;
                TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
                TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
                TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
                TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
                TIM_OC4Init(_tim, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(_tim, TIM_OCPreload_Disable);
                _tim->CCER |= TIM_CC4NP;
                _tim->CCER |= TIM_CC4P;
                _tim->CH4CVR = 10;
                _tim->CCER |= TIM_CC4E;
                

                TIM_ITConfig(_tim, event, ENABLE);
                NVIC_InitTypeDef NVIC_InitStructure;
                NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = priority;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
                NVIC_Init(&NVIC_InitStructure);
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

            if(_channel == 1) {
                _tim->CH1CVR = static_cast<uint16_t>(duty * _arr);
            }
            else if(_channel == 2) {
                _tim->CH2CVR = static_cast<uint16_t>(duty * _arr);
            }
            else if(_channel == 3) {
                _tim->CH3CVR = static_cast<uint16_t>(duty * _arr);
            }
            else if(_channel == 4) {
                _tim->CH4CVR = static_cast<uint16_t>(duty * _arr);
            }
            return true;
        }

        void start() override {
            if(_channel == 1) {
                _tim->CCER |= TIM_CC1E;
            }
            else if(_channel == 2) {
                _tim->CCER |= TIM_CC2E;
            }
            else if(_channel == 3) {
                _tim->CCER |= TIM_CC3E;
            }
            else if(_channel == 4) {
                _tim->CCER |= TIM_CC4E;
            }
        }

        void stop() override {
            if(_channel == 1) {
                _tim->CCER &= ~TIM_CC1E;
            }
            else if(_channel == 2) {
                _tim->CCER &= ~TIM_CC2E;
            }
            else if(_channel == 3) {
                _tim->CCER &= ~TIM_CC3E;
            }
            else if(_channel == 4) {
                _tim->CCER &= ~TIM_CC4E;
            }
        }

        void startComplimentary() {
            if(_channel == 1) {
                _tim->CCER |= TIM_CC1NE;
            }
            else if(_channel == 2) {
                _tim->CCER |= TIM_CC2NE;
            }
            else if(_channel == 3) {
                _tim->CCER |= TIM_CC3NE;
            }
        }

        void stopComplimentary() {
            if(_channel == 1) {
                _tim->CCER &= ~TIM_CC1NE;
            }
            else if(_channel == 2) {
                _tim->CCER &= ~TIM_CC2NE;
            }
            else if(_channel == 3) {
                _tim->CCER &= ~TIM_CC3NE;
            }
        }

        void enable() {
            TIM_Cmd(_tim, ENABLE);
        }

        void disable() {
            TIM_Cmd(_tim, DISABLE);
        }
        
    private:

        u16 _channel;
        u16 _ccp = 0;
        u16 _arr;
        TIM_TypeDef* _tim;
        GPIO_TypeDef* _port;
        u16 _pwmPin;
        u16 _div;
        GPIO_TypeDef* _portN;
        u16 _pwmPinN;
    };

    class TimerBaseCh32v {
    public:
        TimerBaseCh32v(TIM_TypeDef* tim, uint16_t arr = 0xFFFF, uint16_t psc = 72) : _tim(tim) {
            TIM_TimeBaseInitTypeDef TIMER_InitStructure;
            NVIC_InitTypeDef NVIC_InitStructure;

            uint32_t timPeriph  = 0;
            if(tim == TIM1) {
                timPeriph = RCC_APB2Periph_TIM1;
                _irq = TIM1_UP_IRQn;
            }
            else if(tim == TIM2) {
                timPeriph = RCC_APB1Periph_TIM2;
                _irq = TIM2_IRQn;
            }
            else if(tim == TIM3) {
                timPeriph = RCC_APB1Periph_TIM3;
                _irq = TIM3_IRQn;
            }
            else if(tim == TIM4) {
                timPeriph = RCC_APB1Periph_TIM4;
                _irq = TIM4_IRQn;
            }
            else if(tim == TIM5) {
                timPeriph = RCC_APB1Periph_TIM5;
            }

            RCC_APB1PeriphClockCmd(timPeriph, ENABLE);

            TIM_TimeBaseStructInit(&TIMER_InitStructure);
            TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
            TIMER_InitStructure.TIM_Prescaler = psc;
            TIMER_InitStructure.TIM_Period = arr;
            TIMER_InitStructure.TIM_RepetitionCounter = 1;
            TIM_TimeBaseInit(_tim, &TIMER_InitStructure);
            TIM_SetCounter(_tim, 0);
        }

        void setIrq(uint8_t priority, uint16_t event = TIM_IT_Update) {
            NVIC_InitTypeDef NVIC_InitStructure;
            NVIC_InitStructure.NVIC_IRQChannel = _irq;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = priority;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
            TIM_ITConfig(_tim, event, ENABLE);
        }


		uint16_t getCounter() {
            return TIM_GetCounter(_tim);
        }

        void start() {
            TIM_Cmd(_tim, ENABLE);
        }

        void stop() {
            TIM_Cmd(_tim, DISABLE);
        }

    private:
        IRQn _irq;
        TIM_TypeDef* _tim;
    };

    class ComplimentaryPwmCh32v : public IPwm {
    public:
        ComplimentaryPwmCh32v(TIM_TypeDef* tim, GPIO_TypeDef* port, u16 pwmPin, GPIO_TypeDef* portN, u16 pwmPinN, u16 channel, u16 div = TIM_CKD_DIV1, u16 arr = 500) : _tim(tim), _port(port), _pwmPin(pwmPin), _portN(portN), _pwmPinN(pwmPinN), _channel(channel), _div(div), _arr(arr)  {

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

            if(_portN != _port) {
                if(_portN == GPIOB) {
                    rccPeriph |= RCC_APB2Periph_GPIOB;
                }
                else if(_portN == GPIOC) {
                    rccPeriph |= RCC_APB2Periph_GPIOC;
                }
                else if(_portN == GPIOD) {
                    rccPeriph |= RCC_APB2Periph_GPIOD;
                }
                else if(_portN == GPIOE) {
                    rccPeriph |= RCC_APB2Periph_GPIOE;
                }
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
            TIM_BDTRInitTypeDef     TIM_BDTRInitStructure = {0};

            GPIO_InitStructure.GPIO_Pin = pwmPin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(_port, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = pwmPinN;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(_portN, &GPIO_InitStructure);

            TIM_TimeBaseInitStructure.TIM_Period = arr;
            TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;
            TIM_TimeBaseInitStructure.TIM_ClockDivision = div;
            TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
            TIM_TimeBaseInit(_tim, &TIM_TimeBaseInitStructure);

            TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;

            TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
            TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
            TIM_OCInitStructure.TIM_Pulse = 0;
            TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
            TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
            TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
            TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;
            if(channel == 1) {
                TIM_OC1Init(_tim, &TIM_OCInitStructure);
                TIM_OC1PreloadConfig(_tim, TIM_OCPreload_Disable);
            }
            else if(channel == 2) {
                TIM_OC2Init(_tim, &TIM_OCInitStructure);
                TIM_OC2PreloadConfig(_tim, TIM_OCPreload_Disable);
            }
            else if(channel == 3) {
                TIM_OC3Init(_tim, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(_tim, TIM_OCPreload_Disable);
            }
            else if(channel == 4) {
                TIM_OC4Init(_tim, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(_tim, TIM_OCPreload_Disable);
            }

            TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;
            TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;
            TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
            TIM_BDTRInitStructure.TIM_DeadTime = 0xFF;
            TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
            TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
            TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
            TIM_BDTRConfig(_tim, &TIM_BDTRInitStructure);

            TIM_CtrlPWMOutputs(_tim, ENABLE);
            TIM_ARRPreloadConfig(_tim, ENABLE);
            TIM_Cmd(_tim, ENABLE);
        }

        ~ComplimentaryPwmCh32v() {
            disable();
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

        void enable() {
            TIM_Cmd(_tim, ENABLE);
        }

        void disable() {
            TIM_Cmd(_tim, DISABLE);
        }
        

    private:
        u16 _channel;
        u16 _ccp = 0;
        u16 _arr;
        TIM_TypeDef* _tim;
        GPIO_TypeDef* _port;
        GPIO_TypeDef* _portN;
        u16 _pwmPin;
        u16 _pwmPinN;
        u16 _div;
    };
}