#if defined(SYSTICK_ROUTINE)

#include "SystickCh32.h"

uint32_t temp = 3;

using namespace ES::Driver::Systick;

SystickCh32* systickPtr;

SystickCh32::SystickCh32(uint32_t hz,const std::function<void()>& systickCallback, uint8_t interruptPriority) : _systickCallback(systickCallback) {
    while(_systickBusy) {
    } //TODO make assert
    SysTick->CTLR = 0;
    SysTick->SR   = 0;
    SysTick->CNT  = 0;
    SysTick->CMP  = SystemCoreClock / hz;
    SysTick->CTLR = 0xF;
    NVIC_SetPriority(SysTicK_IRQn, interruptPriority);
    NVIC_EnableIRQ(SysTicK_IRQn);
    systickPtr = this;
    
    _systickBusy = true;
}

SystickCh32::~SystickCh32() {
    SysTick->CTLR = 0;
    SysTick->SR   = 0;
    SysTick->CNT  = 0;
    SysTick->CTLR = 0;
    NVIC_DisableIRQ(SysTicK_IRQn);
    systickPtr = nullptr;
    _systickBusy = false;
}

bool SystickCh32::_systickBusy = false;

extern "C" {
void SysTick_Handler(void)  __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
    {   
        SysTick->SR = 0;
        systickPtr->systickRoutine();
    }
}

#endif