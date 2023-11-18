#if defined(DSHOT)

#include "Dshot.h"

ES::Driver::Telemetry::Dhsot::DshotListner* dshotPtr;

namespace ES::Driver::Telemetry::Dhsot {

    DshotListner::DshotListner() {
        dshotPtr = this;
        DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
        
        TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
    }

extern "C" {

    void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void TIM2_IRQHandler(void) {
        if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)
        {   
            TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
            dshotPtr->callbackCc2(TIM_GetCapture2(TIM2));
        }

    }

    void DMA1_Channel5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void DMA1_Channel5_IRQHandler(void) {
        if (DMA_GetITStatus(DMA1_IT_TC5) != RESET)
        {  
            DMA_ClearITPendingBit(DMA1_IT_TC5);
            dshotPtr->callbackDma();
        }
    }

}
}

#endif