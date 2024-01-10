#if defined(ESC)

#include "Esc.h"

    using namespace  ES::Driver::MotorControl;

    Esc6Step* escPtr;

    void Esc6Step::getInstance() {
        escPtr = this;
    }

extern "C" {
    void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void TIM1_UP_IRQHandler(void) {
        if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
        {   
            TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
            escPtr->intTim1Callback();
        }
    }

    void TIM1_CC_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void TIM1_CC_IRQHandler(void) {
        if (TIM_GetITStatus(TIM1, TIM_IT_CC4) != RESET)
        {   
            TIM_ClearITPendingBit(TIM1, TIM_IT_CC4);
            //escPtr->intTim1CC4Callback();
        }
    }



    /*void ADC1_2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void ADC1_2_IRQHandler(void) {
        if (ADC_GetITStatus(ADC1, ADC_IT_JEOC) != RESET)
        {   
            ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
            escPtr->intAdcCallback();
        }
    }*/
}

#endif
