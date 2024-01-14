#if defined CH32V_ADC

#include "AdcCh32v.h"

ES::Driver::Adc::AdcCh32vEsc* adcPtr;

namespace ES::Driver::Adc {

    AdcCh32vEsc::AdcCh32vEsc(ADC_TypeDef* adc, u16 pin1, u16 pin2, const std::function<void(uint16_t inputVoltage, uint16_t current, uint16_t temp)>& handler) : _adc(adc), _adcComplete(handler) {
        adcPtr = this;
        uint32_t adcPeriph  = 0;
        if(_adc == ADC1) {
            adcPeriph = RCC_APB2Periph_ADC1;
        }
        else if(_adc == ADC2) {
            adcPeriph = RCC_APB2Periph_ADC2;
        }

        GPIO_InitTypeDef GPIO_InitStructure = {0};

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(adcPeriph, ENABLE);
        RCC_ADCCLKConfig(RCC_PCLK2_Div2);

        GPIO_InitStructure.GPIO_Pin = pin1 | pin2;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        ADC_DeInit(_adc);
        ADC_InitTypeDef  ADC_InitStructure = {0};
        ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_InitStructure.ADC_ScanConvMode = DISABLE;
        ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
        ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfChannel = 1;
        ADC_Init(_adc, &ADC_InitStructure);

        ADC_InjectedSequencerLengthConfig(_adc, 1);
        ADC_RegularChannelConfig(_adc, ADC_Channel_TempSensor, 1, ADC_SampleTime_7Cycles5);
        //ADC_InjectedChannelConfig(_adc, getChanFromGpio(pin2), 2, ADC_SampleTime_7Cycles5);
        ADC_InjectedChannelConfig(_adc, getChanFromGpio(pin1), 1, ADC_SampleTime_7Cycles5);
        ADC_AutoInjectedConvCmd(_adc, ENABLE);
    }


}

extern "C" {
    /*void DMA1_Channel1_IRQHandler(void)
    {
        if(DMA_GetITStatus(DMA1_IT_TC1))
        {
            DMA_ClearFlag(DMA1_IT_TC1|DMA1_IT_HT1);

            adcPtr->sendParams();

        }

        if(DMA_GetITStatus(DMA1_IT_TE1))
        {
            DMA_ClearFlag(DMA1_IT_TE1);
        }
    }*/

    void ADC1_2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
    void ADC1_2_IRQHandler(void) {
        if (ADC_GetITStatus(ADC1, ADC_IT_JEOC) != RESET)
        {   
            ADC_ClearITPendingBit(ADC1, ADC_IT_JEOC);
            adcPtr->sendParams();
        }
    }
}

#endif