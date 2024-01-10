#if defined(DSHOT)

#include "Dshot.h"

ES::Driver::Telemetry::Dshot::DshotListner* dshotPtr;

namespace ES::Driver::Telemetry::Dshot {

    DshotListner::DshotListner(const std::function<void(uint16_t throttle)>& handler) : _packetRecievedHandler(handler) {
        dshotPtr = this;
        DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
        NVIC_EnableIRQ(DMA1_Channel7_IRQn);
        TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
    }

extern "C" {

    void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void TIM2_IRQHandler(void) {
        if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
        {   
            TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
            dshotPtr->callbackCc1(TIM_GetCapture1(TIM2));
        }

    }

    void DMA1_Channel7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void DMA1_Channel7_IRQHandler(void) {
        if (DMA_GetITStatus(DMA1_IT_TC7) != RESET)
        {  
            DMA_ClearITPendingBit(DMA1_IT_TC7);
            dshotPtr->callbackDma();
        }
    }

}
}

#endif