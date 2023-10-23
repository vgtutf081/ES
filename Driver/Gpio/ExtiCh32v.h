#pragma once

#include "CriticalSection.h"

namespace ES::Driver::Gpio {

    struct InterruptCallback;
    bool configureInterrupt(Ch32vPin* pin, PullMode pullMode, InterruptMode interruptMode, InterruptCallback* callback);
    void releaseInterrupt(InterruptCallback* callback);
    void releaseInterrupt(Ch32vPin* pin);

    uint32_t getExtiLine(Ch32vPin* pin);

    struct InterruptCallback {
		/*virtual ~InterruptCallback(){
			releaseInterrupt(this);
		}*/
		virtual void onGpioInterrupt(Ch32vPin* pin) = 0;
	};

    struct ExtiVector {
        Ch32vPin* pin;
        InterruptCallback* callback = nullptr;
    };

    static ExtiVector _extiVectorTable[16];

    void handleExti(int first, int last){
        for(int i = first; i <= last; ++i){
            const auto& vector = _extiVectorTable[i];
            auto pin = vector.pin->getPin();

            if(EXTI_GetITStatus(pin) != RESET){
                EXTI_ClearITPendingBit(pin);

                bool hasUserCallback = vector.callback != nullptr;

                if(hasUserCallback){
                    vector.callback->onGpioInterrupt(vector.pin);
                }
                else {
                    return;
                }
            }
        }
    }

    void extiClearInterrupt(Ch32vPin* pin) {
        EXTI_ClearITPendingBit(getExtiLine(pin));
    }

    uint8_t getPortSource(Ch32vPin* pin) {
        auto port = pin->getPort();
        if(port == GPIOA) {
            return GPIO_PortSourceGPIOA;
        }
        else if(port == GPIOB) {
            return GPIO_PortSourceGPIOB;
        }
        else if(port == GPIOC) {
            return GPIO_PortSourceGPIOC;
        }
        else if(port == GPIOD) {
            return GPIO_PortSourceGPIOD;
        }
        else if(port == GPIOE) {
            return GPIO_PortSourceGPIOE;
        }
        else if(port == GPIOF) {
            return GPIO_PortSourceGPIOF;
        }
        return 0;
    }
    

    IRQn_Type getInterruptId(Ch32vPin* pin) {

        switch(pin->getPin()) {
        case GPIO_Pin_0:
            return EXTI0_IRQn;
        case GPIO_Pin_1:
            return EXTI1_IRQn;
        case GPIO_Pin_2:
            return EXTI2_IRQn;
        case GPIO_Pin_3:
            return EXTI3_IRQn;
        case GPIO_Pin_4:
            return EXTI4_IRQn;
        case GPIO_Pin_5:
        case GPIO_Pin_6:
        case GPIO_Pin_7:
        case GPIO_Pin_8:
        case GPIO_Pin_9:
            return EXTI9_5_IRQn;
        case GPIO_Pin_10:
        case GPIO_Pin_11:
        case GPIO_Pin_12:
        case GPIO_Pin_13:
        case GPIO_Pin_14:
        case GPIO_Pin_15:
            return EXTI15_10_IRQn;
        default:
            return EXTI0_IRQn;
        }
    }

    uint8_t getPinSource(Ch32vPin* pin) {
        auto _pin = pin->getPin();
        if(_pin == GPIO_Pin_0) {
            return GPIO_PinSource0;
        }
        else if(_pin == GPIO_Pin_1) {
            return GPIO_PinSource1;
        }
        else if(_pin == GPIO_Pin_2) {
            return GPIO_PinSource2;
        }
        else if(_pin == GPIO_Pin_3) {
            return GPIO_PinSource3;
        }
        else if(_pin == GPIO_Pin_4) {
            return GPIO_PinSource4;
        }
        else if(_pin == GPIO_Pin_5) {
            return GPIO_PinSource5;
        }
        else if(_pin == GPIO_Pin_6) {
            return GPIO_PinSource6;
        }
        else if(_pin == GPIO_Pin_7) {
            return GPIO_PinSource7;
        }
        else if(_pin == GPIO_Pin_8) {
            return GPIO_PinSource8;
        }
        else if(_pin == GPIO_Pin_9) {
            return GPIO_PinSource9;
        }
        else if(_pin == GPIO_Pin_10) {
            return GPIO_PinSource10;
        }     
        else if(_pin == GPIO_Pin_11) {
            return GPIO_PinSource11;
        }
        else if(_pin == GPIO_Pin_12) {
            return GPIO_PinSource12;
        }
        else if(_pin == GPIO_Pin_13) {
            return GPIO_PinSource13;
        }
        else if(_pin == GPIO_Pin_14) {
            return GPIO_PinSource14;
        }
        else if(_pin == GPIO_Pin_15) {
            return GPIO_PinSource15;
        }
        return 0;
    }

    uint32_t getExtiLine(Ch32vPin* pin) {
        auto _pin = pin->getPin();
        if(_pin == GPIO_Pin_0) {
            return EXTI_Line0;
        }
        else if(_pin == GPIO_Pin_1) {
            return EXTI_Line1;
        }
        else if(_pin == GPIO_Pin_2) {
            return EXTI_Line2;
        }
        else if(_pin == GPIO_Pin_3) {
            return EXTI_Line3;
        }
        else if(_pin == GPIO_Pin_4) {
            return EXTI_Line4;
        }
        else if(_pin == GPIO_Pin_5) {
            return EXTI_Line5;
        }
        else if(_pin == GPIO_Pin_6) {
            return EXTI_Line6;
        }
        else if(_pin == GPIO_Pin_7) {
            return EXTI_Line7;
        }
        else if(_pin == GPIO_Pin_8) {
            return EXTI_Line8;
        }
        else if(_pin == GPIO_Pin_9) {
            return EXTI_Line9;
        }
        else if(_pin == GPIO_Pin_10) {
            return EXTI_Line10;
        }     
        else if(_pin == GPIO_Pin_11) {
            return EXTI_Line11;
        }
        else if(_pin == GPIO_Pin_12) {
            return EXTI_Line12;
        }
        else if(_pin == GPIO_Pin_13) {
            return EXTI_Line13;
        }
        else if(_pin == GPIO_Pin_14) {
            return EXTI_Line14;
        }
        else if(_pin == GPIO_Pin_15) {
            return EXTI_Line15;
        }
        return 0;
    }

    void extiConfiguteCallback(Ch32vPin* pin, InterruptCallback* callback){
        if(callback == nullptr) {
            return;
        }

        Threading::CriticalSection lock;
        if(_extiVectorTable[getPinSource(pin)].callback != nullptr){
            return;
        }

        _extiVectorTable[getPinSource(pin)].callback = callback;
        _extiVectorTable[getPinSource(pin)].pin = pin;
    }

    
    void configureInterrupt(Ch32vPin* pin, InterruptCallback* callback, InterruptMode trigger = InterruptMode::Falling) {
        GPIO_InitTypeDef GPIO_InitStructure = {0};
        EXTI_InitTypeDef EXTI_InitStructure = {0};
        NVIC_InitTypeDef NVIC_InitStructure = {0};

        extiConfiguteCallback(pin, callback);

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
        
        GPIOMode_TypeDef mode;
        EXTITrigger_TypeDef trigMode;
        if(trigger == InterruptMode::Falling) {
            mode = GPIO_Mode_IPU;
            trigMode = EXTI_Trigger_Falling;
        }
        else if(trigger == InterruptMode::Rising) {
            mode = GPIO_Mode_IPD;
            trigMode = EXTI_Trigger_Rising;
        }
        else {
            mode = GPIO_Mode_IN_FLOATING;
            trigMode = EXTI_Trigger_Rising_Falling;
        }
        GPIO_InitStructure.GPIO_Pin = pin->getPin();
        GPIO_InitStructure.GPIO_Mode = mode;
        GPIO_Init(pin->getPort(), &GPIO_InitStructure);

        GPIO_EXTILineConfig(getPortSource(pin), getPinSource(pin));
        EXTI_InitStructure.EXTI_Line = getExtiLine(pin);
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = trigMode;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = getInterruptId(pin);
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }

}

	extern "C" void EXTI0_IRQHandler(){ ES::Driver::Gpio::handleExti(0, 0); }
	extern "C" void EXTI1_IRQHandler(){ ES::Driver::Gpio::handleExti(1, 1); }
	extern "C" void EXTI2_IRQHandler(){ ES::Driver::Gpio::handleExti(2, 2); }
	extern "C" void EXTI3_IRQHandler(){ ES::Driver::Gpio::handleExti(3, 3); }
	extern "C" void EXTI4_IRQHandler(){ ES::Driver::Gpio::handleExti(4, 4); }
	extern "C" void EXTI9_5_IRQHandler(){
         ES::Driver::Gpio::handleExti(5, 9); }
	extern "C" void EXTI15_10_IRQHandler(){ ES::Driver::Gpio::handleExti(10, 15); }