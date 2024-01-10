#pragma once

#include "ch32v20x_gpio.h"

namespace ES::Driver::Comparator {

#if defined CH32V203F8U6

#define BEMFC_PIN GPIO_Pin_5
#define BEMFB_PIN GPIO_Pin_10
#define BEMFA_PIN GPIO_Pin_11
#define BEMFCOMP_PIN1 GPIO_Pin_15
#define BEMFCOMP_PIN2 GPIO_Pin_14
#define COMP_OUT1 GPIO_Pin_2
#define COMP_OUT2 GPIO_Pin_3
#define BEMFC_GPIOBUS GPIOA
#define BEMFB_GPIOBUS GPIOB
#define BEMFA_GPIOBUS GPIOB
#define BEMFCOMP_GPIOBUS GPIOB
#define COMPOUT_GPIOBUS GPIOA

#define EXTI_PIN_SOURCE GPIO_PinSource2
#define EXTI_PORT_SOURSE GPIO_PortSourceGPIOA
#define EXTI_LINE EXTI_Line2
#define EXTI_IRQ EXTI2_IRQn




    enum class CompInstance {
        Opa1Out0,
        Opa2In0Out0,
        Opa2In1Out0,
    };
    
    class BemfComparators {
    public:
        BemfComparators() {
            while(_opaBusy) {
            }

            GPIO_InitTypeDef        GPIO_InitStruct = {0};
            EXTI_InitTypeDef        EXTI_InitStructure = {0};

            GPIO_InitStruct.GPIO_Pin  = BEMFC_PIN;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(BEMFC_GPIOBUS, &GPIO_InitStruct);

            GPIO_InitStruct.GPIO_Pin  = BEMFB_PIN;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(BEMFB_GPIOBUS, &GPIO_InitStruct);
            
            GPIO_InitStruct.GPIO_Pin  = BEMFA_PIN;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(BEMFA_GPIOBUS, &GPIO_InitStruct);

            GPIO_InitStruct.GPIO_Pin  = BEMFCOMP_PIN2 | BEMFCOMP_PIN1;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(BEMFCOMP_GPIOBUS, &GPIO_InitStruct);


            GPIO_InitStruct.GPIO_Pin  = COMP_OUT1 | COMP_OUT2;
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
            GPIO_Init(COMPOUT_GPIOBUS, &GPIO_InitStruct);
            
            OPA->CR = 0x01;

            GPIO_EXTILineConfig(EXTI_PORT_SOURSE, EXTI_PIN_SOURCE);
            EXTI_InitStructure.EXTI_Line = EXTI_LINE;
            EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
            EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
            EXTI_InitStructure.EXTI_LineCmd = DISABLE;
            EXTI_Init(&EXTI_InitStructure);
            EXTI_ClearITPendingBit(EXTI_LINE);

            NVIC_SetPriority(EXTI_IRQ, 0x0);
            NVIC_EnableIRQ(EXTI_IRQ);
            //extern void EXTI2_IRQHandler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
            //SetVTFIRQ((uint32_t)EXTI2_IRQHandler, EXTI2_IRQn, 1, ENABLE);

            _opaBusy = true;
        }

        ~BemfComparators() {
            NVIC_DisableIRQ(EXTI_IRQ);
            OPA->CR = 0x00;

            _opaBusy = false;
        }

        void setOutput(CompInstance instance) {
            if(instance == CompInstance::Opa1Out0) {
                OPA->CR = 0x01;
                return;
            }
            if(instance == CompInstance::Opa2In0Out0) {
                OPA->CR = 0x10;
                return;
            }
            if(instance == CompInstance::Opa2In1Out0) {
                OPA->CR = 0x50;
                return;
            }
        }

        void enableInt() {
            EXTI->INTENR |= (1 << EXTI_PIN_SOURCE);
        }

        void maskInt() {
            EXTI->INTENR &= ~(1 << EXTI_PIN_SOURCE);
            EXTI->INTFR  = (1 << EXTI_PIN_SOURCE); 
        }

        bool getOutputState() {
            return COMPOUT_GPIOBUS->INDR & COMP_OUT1;
        }
    
    private:
        static bool _opaBusy;
    };

#endif

}