#pragma once

#include "ch32v20x_tim.h"
#include "ch32v20x_rcc.h"
#include "ITimer.h"

#include <limits>
#include <cstdint>

namespace ES::Driver::Timer { 

    const uint16_t TimerMax = 0xFFFF;

    class TimerBaseCh32v {
    public:
        TimerBaseCh32v(TIM_TypeDef* tim, uint16_t arr = 0xFFFF, uint16_t psc = 1, uint16_t clockDiv = TIM_CKD_DIV1) : _tim(tim), _arr(arr), _psc(psc), _div(clockDiv) {
            TIM_TimeBaseInitTypeDef TIMER_InitStructure;

            
            RCC_ClocksTypeDef rcc;
            RCC_GetClocksFreq(&rcc);
            if(_tim == TIM1) {
                _timBaseFreq = rcc.PCLK2_Frequency;
                if(rcc.PCLK2_Frequency != rcc.HCLK_Frequency) {
                    _timBaseFreq = _timBaseFreq * 2;
                }
            }
            else {
                _timBaseFreq = rcc.PCLK1_Frequency;
                if(rcc.PCLK1_Frequency != rcc.HCLK_Frequency) {
                    _timBaseFreq = _timBaseFreq * 2;
                }
            }
            if(_div == TIM_CKD_DIV2) {
                _timBaseFreq /= 2;
            }
            else if(_div == TIM_CKD_DIV4) {
                _timBaseFreq /= 4;
            }

            uint32_t timPeriph  = 0;
            if(tim == TIM1) {
                timPeriph = RCC_APB2Periph_TIM1;
                _irq = TIM1_UP_IRQn;
                RCC_APB2PeriphClockCmd(timPeriph, ENABLE);
            }
            else {
            if(tim == TIM2) {
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
            }

            TIM_TimeBaseStructInit(&TIMER_InitStructure);
            TIMER_InitStructure.TIM_ClockDivision = _div;
            TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
            TIMER_InitStructure.TIM_Prescaler = psc - 1;
            TIMER_InitStructure.TIM_Period = arr;

            TIM_TimeBaseInit(_tim, &TIMER_InitStructure);
            TIM_ARRPreloadConfig( _tim, ENABLE );
            TIM_Cmd( TIM1, ENABLE );
        } 

        TimerBaseCh32v() {
        }

        void setIrq(uint8_t priority = 0) {
            NVIC_SetPriority(_irq, priority);
            NVIC_EnableIRQ(_irq);
        }

        void disableIrq() {
            TIM_ITConfig(_tim, _event, DISABLE);
        }

        void enableIrq(uint16_t event = TIM_IT_Update) {
            _event = event;
            TIM_ITConfig(_tim, _event, ENABLE);
        }

        void clearIrq() {
            TIM_ClearITPendingBit(_tim, _event);
        }

        bool setFreq(u32 freqHz) {
            int32_t prescaler = 1;
            prescaler = (_timBaseFreq / freqHz) / _arr;
            if(prescaler > std::numeric_limits<u16>::max() || prescaler < 0) {
                return false;
            }
            setPrescaler(static_cast<uint16_t>(prescaler));
            return false;
        }

        void setUpdateTimeMs(uint32_t timeUs) {
            uint32_t baseUpTimeNs = 1000000000u / (_timBaseFreq / _arr);
            uint64_t targetTimeNs = timeUs * 1000u;
            if(baseUpTimeNs > targetTimeNs) {
                uint32_t tickNs = 1000000000u / _timBaseFreq;
                setPeriod(targetTimeNs / tickNs);
            }   
            else if(baseUpTimeNs == targetTimeNs){
                setPrescaler(1);
                return;
            }
            else {
                uint16_t psc = targetTimeNs / baseUpTimeNs;
                setPrescaler(psc);
                uint32_t currentTicksInNs = 1000000000u / (_timBaseFreq / psc);
                setPeriod(targetTimeNs / currentTicksInNs);
                asm("nop");
            }
        }

        void bdtrConfig(uint16_t deadTime = 0) {
            TIM_BDTRInitTypeDef     TIM_BDTRInitStructure = {0};
            TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;
            TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;
            TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
            TIM_BDTRInitStructure.TIM_DeadTime = deadTime;
            TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
            TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;
            TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;
            TIM_BDTRConfig(_tim, &TIM_BDTRInitStructure);
        }

        ~TimerBaseCh32v() {
            stop();
            TIM_DeInit(_tim);

        }

        void setOutputTrigger(uint16_t ccValue, uint16_t trigger = TIM_TRGOSource_OC4Ref) {
            if(trigger == TIM_TRGOSource_OC4Ref) {
                TIM_OCInitTypeDef TIM_OCInitStructure={0};
                TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
                TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
                TIM_OCInitStructure.TIM_Pulse = ccValue;
                TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
                TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
                TIM_OC4Init(_tim, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(_tim, TIM_OCPreload_Disable);
                TIM_SelectOutputTrigger(_tim, trigger);
                NVIC_InitTypeDef NVIC_InitStructure;
                NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
                NVIC_Init(&NVIC_InitStructure);
                TIM_ITConfig(_tim, TIM_IT_CC4, ENABLE);
            }
        }

        void configCC4() {
            TIM_OCInitTypeDef TIM_OCInitStructure={0};
            TIM_OC4PreloadConfig(_tim, TIM_OCPreload_Enable);
            TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Disable;
            TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
            TIM_OC4Init( _tim, &TIM_OCInitStructure );
            TIM_OC4FastConfig(_tim,TIM_OCFast_Disable);

            TIM_CCxCmd(_tim, TIM_Channel_4, TIM_CCx_Enable);
            _tim->CH4CVR = 100;    
        }

        void triggerEn() {
            _tim->CCER |= TIM_CC4E;
        }
        void triggerDisable() {
            _tim->CCER &= ~TIM_CC4E;
        }

        void setPeriod(uint16_t period) {
            stop();
            setCounter(0);
            if(period > 0xFFFF) {
                _arr = 0xFFFF;
            }
            else {
                _arr = period;
            }
            _tim->ATRLR = _arr;
            start();
        }

        void setCounter(uint16_t counter) {
            return TIM_SetCounter(_tim, counter);
        }

        void setPrescaler(uint16_t psc) {
            _psc = psc;
            _tim->PSC = psc - 1;
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

        void enableCtrlPwmOutputs() {
            TIM_CtrlPWMOutputs(_tim, ENABLE);
        }

        void generateUpdateEvent() {
            TIM_GenerateEvent(_tim, TIM_EventSource_Update);
        }

        TIM_TypeDef* getTimPointer() {
            return _tim;
        }

        uint16_t getIrqEvent() {
            return _irqEvent;
        }

        uint16_t getArr() {
            return _arr;
        }

        uint16_t getDiv() {
            return _div;
        }

        uint16_t getPeriod() {
            return _tim->ATRLR;
        }

        uint32_t getTimBaseFreq() {
            return _timBaseFreq;
        }

    private:

        uint16_t _event;
        u32 _timBaseFreq;
        u16 _div;
        u16 _psc;
        uint16_t _arr;
        uint16_t _irqEvent;
        IRQn _irq;
        TIM_TypeDef* _tim;
    };

    class PwmCh32v {
    public:
        PwmCh32v(TimerBaseCh32v& tim, GPIO_TypeDef* port, u16 pwmPin, u16 channel) : _port(port), _pwmPin(pwmPin), _channel(channel) {

            _tim = tim.getTimPointer();

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

            RCC_APB2PeriphClockCmd(rccPeriph, ENABLE);

            GPIO_InitTypeDef GPIO_InitStructure={0};
            TIM_OCInitTypeDef TIM_OCInitStructure={0};

            GPIO_InitStructure.GPIO_Pin = pwmPin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
            GPIO_Init(port, &GPIO_InitStructure);

            TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
            TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
            TIM_OCInitStructure.TIM_Pulse = 0;
            TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
            TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
            if(_channel == 1) {
                TIM_OC1Init(_tim, &TIM_OCInitStructure);
                TIM_OC1PreloadConfig(_tim, TIM_OCPreload_Enable);
                TIM_OC1FastConfig(TIM1,TIM_OCFast_Disable);
            }
            if(_channel == 2) {
                TIM_OC2Init(_tim, &TIM_OCInitStructure);
                TIM_OC2PreloadConfig(_tim, TIM_OCPreload_Enable);
                TIM_OC2FastConfig(TIM1,TIM_OCFast_Disable);
            }
            if(_channel == 3) {
                TIM_OC3Init(_tim, &TIM_OCInitStructure);
                TIM_OC3PreloadConfig(_tim, TIM_OCPreload_Enable);
                TIM_OC3FastConfig(TIM1,TIM_OCFast_Disable);
            }
            if(_channel == 4) {
                TIM_OC4Init(_tim, &TIM_OCInitStructure);
                TIM_OC4PreloadConfig(_tim, TIM_OCPreload_Enable);
                TIM_OC4FastConfig(TIM1,TIM_OCFast_Disable);
            }
            TIM_CtrlPWMOutputs(_tim, ENABLE);
        }

        PwmCh32v() {

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

            TIM_OCInitTypeDef TIM_OCInitStructure={0};
            TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
            TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
            TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
            TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
            TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
            TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
            TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;

            if(_channel == 1) {
                TIM_OC1Init(_tim, &TIM_OCInitStructure);
                TIM_OC1FastConfig(TIM1,TIM_OCFast_Disable);
                TIM_OC1PreloadConfig(_tim, TIM_OCPreload_Enable);
            }
            else if(_channel == 2) {
                TIM_OC2Init(_tim, &TIM_OCInitStructure);
                TIM_OC2FastConfig(TIM1,TIM_OCFast_Disable);
                TIM_OC2PreloadConfig(_tim, TIM_OCPreload_Enable);
            }
            else if(_channel == 3) {
                TIM_OC3Init(_tim, &TIM_OCInitStructure);
                TIM_OC3FastConfig(TIM1,TIM_OCFast_Disable);
                TIM_OC3PreloadConfig(_tim, TIM_OCPreload_Enable);
            }
        }

        bool setParams(float duty) {
            _ccp = static_cast<uint16_t>(duty * _tim->ATRLR);

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
            return true;
        }

        bool setCompare(uint16_t value) {
            _ccp = value;
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
            return true;
        }

        void start() {
            if(_channel == 1) {
                _tim->CCER |= TIM_CC1E;
                //_tim->CCER |= TIM_CC1NE;
            }
            else if(_channel == 2) {
                _tim->CCER |= TIM_CC2E;
                //_tim->CCER |= TIM_CC3NE;
            }
            else if(_channel == 3) {
                _tim->CCER |= TIM_CC3E;
                //_tim->CCER |= TIM_CC3NE;
            }
            else if(_channel == 4) {
                _tim->CCER |= TIM_CC4E;
            }
        }

        void stop() {
            if(_channel == 1) {
                _tim->CCER &= ~TIM_CC1E;
                //_tim->CCER &= ~TIM_CC1NE;
            }
            else if(_channel == 2) {
                _tim->CCER &= ~TIM_CC2E;
                //_tim->CCER &= ~TIM_CC2NE;
            }
            else if(_channel == 3) {
                _tim->CCER &= ~TIM_CC3E;
                //_tim->CCER &= ~TIM_CC3NE;
            }
            else if(_channel == 4) {
                _tim->CCER &= ~TIM_CC4E;
            }
        }

        void startComplimentary() {
            if(_channel == 1) {
                //_tim->CH1CVR = 0xFFFF;
                _tim->CCER |= TIM_CC1NE;

            }
            else if(_channel == 2) {
                //_tim->CH2CVR = 0xFFFF;
                _tim->CCER |= TIM_CC2NE;

            }
            else if(_channel == 3) {
                //_tim->CH3CVR = 0xFFFF;
                _tim->CCER |= TIM_CC3NE;

            }
        }

        void stopComplimentary() {
            if(_channel == 1) {
                _tim->CH1CVR = _ccp;
                _tim->CCER &= ~TIM_CC1NE;

            }
            else if(_channel == 2) {
                _tim->CH2CVR = _ccp;
                _tim->CCER &= ~TIM_CC2NE;

            }
            else if(_channel == 3) {
                _tim->CH3CVR = _ccp;
                _tim->CCER &= ~TIM_CC3NE;

            }

        }
        
    private:

        u16 _channel;
        u16 _ccp = 0;
        TIM_TypeDef* _tim;
        GPIO_TypeDef* _port;
        u16 _pwmPin;
        GPIO_TypeDef* _portN;
        u16 _pwmPinN;
    };

    class TimerInputCapture {
    public:
        TimerInputCapture(TIM_TypeDef* tim, GPIO_TypeDef* port, u16 pin, u16 channel, u32 memadr) : _tim(tim), _port(port), _pin(pin), _channel(channel) {

            uint32_t timPeriph  = 0;
            if(tim == TIM1) {
                timPeriph = RCC_APB2Periph_TIM1;
                _irq = TIM1_UP_IRQn;
                RCC_APB2PeriphClockCmd(timPeriph, ENABLE);
            }
            else {
            if(tim == TIM2) {
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
            }

            uint32_t rccPeriph  = 0;
            if(port == GPIOA) {
                rccPeriph = RCC_APB2Periph_GPIOA;
            }
            else if(port == GPIOB) {
                rccPeriph = RCC_APB2Periph_GPIOB;
            }
            else if(port == GPIOC) {
                rccPeriph = RCC_APB2Periph_GPIOC;
            }
            else if(port == GPIOD) {
                rccPeriph = RCC_APB2Periph_GPIOD;
            }
            else if(port == GPIOE) {
                rccPeriph = RCC_APB2Periph_GPIOE;
            }

            RCC_APB2PeriphClockCmd(rccPeriph, ENABLE);

            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

            GPIO_InitTypeDef        GPIO_InitStructure = {0};
            TIM_ICInitTypeDef       TIM_ICInitStructure = {0};
            TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
            NVIC_InitTypeDef        NVIC_InitStructure = {0};

            GPIO_InitStructure.GPIO_Pin = pin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(port, &GPIO_InitStructure);
            GPIO_ResetBits(port, pin);

            TIM_TimeBaseInitStructure.TIM_Period = 0xFFFF;
            TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
            TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
            TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;

            TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0x00;
            TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

            TIM_ICInitStructure.TIM_Channel = channel;
            TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
            TIM_ICInitStructure.TIM_ICFilter = 0x00;
            TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
            TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;

            TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);

            NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x20;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1);
            TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
            TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Disable);

            DMA_InitTypeDef DMA_InitStructure = {0};
            /*DMA_DeInit(DMA1_Channel7);
            DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40000038;
            DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
            DMA_InitStructure.DMA_BufferSize = 16;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
            DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
            DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
            DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
            DMA_Init(DMA1_Channel7, &DMA_InitStructure);

            DMA_Cmd(DMA1_Channel7, ENABLE);*/

            NVIC_InitStructure.NVIC_IRQChannel = _irq;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);

            DMA_DeInit(DMA1_Channel7);
            DMA_InitStructure.DMA_PeripheralBaseAddr = 0x40000038;
            DMA_InitStructure.DMA_MemoryBaseAddr = memadr;
            DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
            DMA_InitStructure.DMA_BufferSize = 16;
            DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
            DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
            DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
            DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
            DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
            DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
            DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
            DMA_Init(DMA1_Channel7, &DMA_InitStructure);

            DMA_Cmd(DMA1_Channel7, ENABLE);

            //DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);

            //TIM_DMACmd(TIM2, TIM_DMA_CC1 | TIM_DMA_CC2, ENABLE);

            //TIM_Cmd(TIM2, ENABLE);
        } 

        void start() {
            TIM_Cmd(TIM2, ENABLE);
        }

        void stop() {
            TIM_Cmd(TIM2, DISABLE);
        }

        TimerInputCapture() {
            
        }

        void setIrq(uint8_t priority, uint16_t event = TIM_IT_Update) {
            TIM_ClearITPendingBit(_tim, event);
            _irqEvent = event;
            if(event == TIM_IT_Update) {
                NVIC_InitTypeDef NVIC_InitStructure;
                NVIC_InitStructure.NVIC_IRQChannel = _irq;
                NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = priority;
                NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
                NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
                NVIC_Init(&NVIC_InitStructure);
                TIM_ITConfig(_tim, event, ENABLE);
            }
        }

    private:
        u16 _channel;
        u16 _pin;
        GPIO_TypeDef* _port;
        uint16_t _irqEvent;
        IRQn _irq;
        TIM_TypeDef* _tim;
    };
}
