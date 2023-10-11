#pragma once

#include "IAdc.h"
#include "ch32v20x.h"
#include "ch32v20x_adc.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_gpio.h"

namespace ES::Driver::Adc {

    class AdcCh32vSingleEnded : public IAdc {
    public:
        AdcCh32vSingleEnded(ADC_TypeDef* adc, GPIO_TypeDef* port, u16 adcPin, u16 channel) : _adc(adc), _port(port), _adcPin(adcPin), _channel(channel) {

            /*uint32_t rccPeriph  = 0;
            if(_port == GPIOA) {
                rccPeriph = RCC_APB2Periph_GPIOA;
            }
            else if(_port == GPIOB) {
                rccPeriph = RCC_APB2Periph_GPIOB;
            }
            else if(_port == GPIOC) {
                rccPeriph = RCC_APB2Periph_GPIOC;
            }
            else if(_port == GPIOD) {
                rccPeriph = RCC_APB2Periph_GPIOD;
            }
            else if(_port == GPIOE) {
                rccPeriph = RCC_APB2Periph_GPIOE;
            }

            uint32_t adcPeriph  = 0;
            if(_adc == ADC1) {
                adcPeriph = RCC_APB2Periph_ADC1;
            }
            else if(_adc == ADC2) {
                adcPeriph = RCC_APB2Periph_ADC2;
            }

            RCC_APB2PeriphClockCmd(rccPeriph, ENABLE);
            RCC_APB2PeriphClockCmd(adcPeriph, ENABLE);
            RCC_ADCCLKConfig(RCC_PCLK2_Div2);

            
            GPIO_InitTypeDef GPIO_InitStructure = {0};
            GPIO_InitStructure.GPIO_Pin = _adcPin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(_port, &GPIO_InitStructure);

            ADC_DeInit(_adc);
            ADC_InitTypeDef  ADC_InitStructure = {0};
            ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
            ADC_InitStructure.ADC_ScanConvMode = DISABLE;
            ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
            ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
            ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
            ADC_InitStructure.ADC_NbrOfChannel = 3;
            ADC_Init(_adc, &ADC_InitStructure);
            enable();
            calibration();*/


        }

        void init(uint8_t rank = 1, uint8_t sampleTime = ADC_SampleTime_41Cycles5) {
                            ADC_InitTypeDef  ADC_InitStructure = {0};
            GPIO_InitTypeDef GPIO_InitStructure = {0};

            RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
            RCC_ADCCLKConfig(RCC_PCLK2_Div2);

            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            ADC_DeInit(ADC1);
            ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
            ADC_InitStructure.ADC_ScanConvMode = ENABLE;
            ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
            ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigInjecConv_T1_CC4;
            ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
            ADC_InitStructure.ADC_NbrOfChannel = 3;
            ADC_Init(ADC1, &ADC_InitStructure);

            ADC_InjectedSequencerLengthConfig(ADC1, 3);
            ADC_InjectedChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_71Cycles5);
            ADC_InjectedChannelConfig(ADC1, ADC_Channel_3, 2, ADC_SampleTime_71Cycles5);
            ADC_InjectedChannelConfig(ADC1, ADC_Channel_4, 3, ADC_SampleTime_71Cycles5);

            ADC_DiscModeChannelCountConfig(ADC1, 1);
            //ADC_InjectedDiscModeCmd(ADC1, ENABLE);
            ADC_ExternalTrigInjectedConvCmd(ADC1, ENABLE);
            ADC_Cmd(ADC1, ENABLE);
            

            ADC_BufferCmd(ADC1, DISABLE); //disable buffer
            ADC_ResetCalibration(ADC1);
            while(ADC_GetResetCalibrationStatus(ADC1));
            ADC_StartCalibration(ADC1);
            while(ADC_GetCalibrationStatus(ADC1));
            _calibrattionValue = Get_CalibrationValue(ADC1);
            ADC_Cmd(ADC1, DISABLE);
        }

        void init2() {
            ADC_Cmd(ADC1, ENABLE);
        }

        u16 getRawValue() {
            u16 value;
            ADC_SoftwareStartConvCmd(_adc, ENABLE);
            while(!ADC_GetFlagStatus(_adc, ADC_FLAG_EOC));
            value = ADC_GetConversionValue(_adc);
            return getConversionValue(value);
        }

        u16 getVoltageMv(float rHighSide, float rLowSide) {
            u16 rawValue = getRawValue();
            u16 value = (rawValue * _vdd) / _bitDepth;
            value = (value * rLowSide) / (rHighSide + rLowSide);
            return value;
        }

        void deInit() {
            disable();
            ADC_DeInit(_adc);
        }

        ~AdcCh32vSingleEnded() {
            deInit();
        }

    private:

        void calibration() {
            ADC_BufferCmd(_adc, DISABLE);
            ADC_ResetCalibration(_adc);
            while(ADC_GetResetCalibrationStatus(_adc));
            ADC_StartCalibration(_adc);
            while(ADC_GetCalibrationStatus(_adc));
            _calibrattionValue = Get_CalibrationValue(_adc);
        }

        u16 getConversionValue(s16 val) {
            if((val + _calibrattionValue) < 0)
                return 0;
            if((_calibrattionValue + val) > 4095||val==4095)
                return 4095;
            return (val + _calibrattionValue);
        }

        void enable() {
            ADC_Cmd(_adc, ENABLE);
        }

        void disable() {
            ADC_Cmd(_adc, DISABLE);
        }
        
        u16 _vdd = 3300;
        u16 _bitDepth = 4095;
        ADC_TypeDef* _adc;
        s16 _calibrattionValue = 0;
        uint8_t _channel; 
        GPIO_TypeDef* _port;
        u16 _adcPin;
    };
}
