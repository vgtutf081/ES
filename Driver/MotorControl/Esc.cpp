#if defined(ESC)

#include "Esc.h"

    using namespace  ES::Driver::MotorControl;

    Esc6Step* escPtr;

    void Esc6Step::getInstance() {
        escPtr = this;
    }

extern "C" {
    void TIM1_UP_IRQHandler(void) __attribute__((interrupt()));
    void TIM1_UP_IRQHandler(void) {
        if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
        {   
            TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
            escPtr->intTim1Callback();
        }
    }

    void TIM4_IRQHandler(void) __attribute__((interrupt()));
    void TIM4_IRQHandler(void) {
        if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
        {   
            TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
            escPtr->intTim4Callback();
        }
    }

    void TIM3_IRQHandler(void) __attribute__((interrupt()));
    void TIM3_IRQHandler(void) {
        if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
        {   
            TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
            escPtr->intTim3Callback();
        }
    }
}

#endif