#if defined(DSHOT)

#include "Dshot.h"

ES::Driver::Telemetry::Dhsot::DshotListner* dshotPtr;

namespace ES::Driver::Telemetry::Dhsot {

    DshotListner::DshotListner() {
        dshotPtr = this;
    }

extern "C" {

    void TIM2_IRQHandler(void) __attribute__((interrupt()));
    void TIM2_IRQHandler(void) {
        if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
        {   
            dshotPtr->callbackCc1(TIM_GetCapture1(TIM2));
        }
        if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)
        {   
            dshotPtr->callbackCc2(TIM_GetCapture2(TIM2));
        }

        TIM_ClearITPendingBit(TIM2, TIM_IT_CC1 | TIM_IT_CC2);
    }

}
}

#endif