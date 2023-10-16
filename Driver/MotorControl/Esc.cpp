#if defined(ESC)

#include "Esc.h"

    using namespace  ES::Driver::MotorControl;

    Esc6Step* ptr;

    void Esc6Step::getInstance() {
        ptr = this;
    }

extern "C" {
    void TIM1_UP_IRQHandler(void) __attribute__((interrupt()));
    void TIM1_UP_IRQHandler(void) {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        ptr->intTim1Callback();
    }

    void TIM2_IRQHandler(void) __attribute__((interrupt()));
    void TIM2_IRQHandler(void) {
        if (TIM_GetITStatus(timMeas, TIM_IT_Update) != RESET)
        {   
            TIM_ClearITPendingBit(timMeas, TIM_IT_Update);
        }
    }

    void TIM3_IRQHandler(void) __attribute__((interrupt()));
    void TIM3_IRQHandler(void) {
        if (TIM_GetITStatus(timComm, TIM_IT_Update) != RESET)
        {   
            ptr->intTim3Callback();
        }
    }
}

#endif