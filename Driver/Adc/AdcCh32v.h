#pragma once

#include "IAdc.h"
#include "ch32v20x.h"
#include "ch32v20x_adc.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_gpio.h"

namespace ES::Driver::Adc {



    class AdcCh32vThreePhaseInj {
    public:
        AdcCh32vThreePhaseInj(ADC_TypeDef* adc, u16 firstRankPin, u16 secondRankPin, u16 thirdRankPin, uint32_t trigConv = ADC_ExternalTrigConv_None) : _adc(adc) {

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

            GPIO_InitStructure.GPIO_Pin = firstRankPin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = secondRankPin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            GPIO_InitStructure.GPIO_Pin = thirdRankPin;
            GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
            GPIO_Init(GPIOA, &GPIO_InitStructure);

            ADC_DeInit(_adc);
            ADC_InitTypeDef  ADC_InitStructure = {0};
            ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
            ADC_InitStructure.ADC_ScanConvMode = ENABLE;
            ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
            ADC_InitStructure.ADC_ExternalTrigConv = trigConv;
            ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
            ADC_InitStructure.ADC_NbrOfChannel = 3;
            ADC_Init(_adc, &ADC_InitStructure);

            ADC_InjectedSequencerLengthConfig(_adc, 3);
            ADC_InjectedChannelConfig(_adc, getChanFromGpio(firstRankPin), 1, ADC_SampleTime_71Cycles5);
            ADC_InjectedChannelConfig(_adc, getChanFromGpio(secondRankPin), 2, ADC_SampleTime_71Cycles5);
            ADC_InjectedChannelConfig(_adc, getChanFromGpio(thirdRankPin), 3, ADC_SampleTime_71Cycles5);

            if(trigConv != ADC_ExternalTrigConv_None) {
                ADC_ExternalTrigInjectedConvCmd(_adc, ENABLE);
            }
            ADC_Cmd(_adc, ENABLE);

            ADC_BufferCmd(_adc, DISABLE); //disable buffer
            ADC_ResetCalibration(_adc);
            while(ADC_GetResetCalibrationStatus(_adc));
            ADC_StartCalibration(_adc);
            while(ADC_GetCalibrationStatus(_adc));
            _calibrattionValue = Get_CalibrationValue(_adc);
            ADC_Cmd(_adc, DISABLE);
        }
        
        AdcCh32vThreePhaseInj() {
            
        }

        uint8_t getChanFromGpio(u16 pin) {
            if(pin == GPIO_Pin_0) {
                return 0;
            }
            if(pin == GPIO_Pin_1) {
                return 1;
            }
            if(pin == GPIO_Pin_2) {
                return 2;
            }
            if(pin == GPIO_Pin_3) {
                return 3;
            }
            if(pin == GPIO_Pin_4) {
                return 4;
            }
            if(pin == GPIO_Pin_5) {
                return 5;
            }
            if(pin == GPIO_Pin_6) {
                return 6;
            }
            if(pin == GPIO_Pin_7) {
                return 7;
            }
            return 0;
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

        ~AdcCh32vThreePhaseInj() {
            deInit();
        }

        ADC_TypeDef* getAdcPtr() {
            return _adc;
        }

        void enable() {
            ADC_Cmd(_adc, ENABLE);
        }

        void disable() {
            ADC_Cmd(_adc, DISABLE);
        }

        u16 getConversionValue(s16 val) {
            if((val + _calibrattionValue) < 0)
                return 0;
            if((_calibrattionValue + val) > 4095||val==4095)
                return 4095;
            return (val + _calibrattionValue);
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

        u16 _vdd = 3300;
        u16 _bitDepth = 4095;
        ADC_TypeDef* _adc;
        s16 _calibrattionValue = 0;
    };

    class AdcCh32vRegularChannel {
    public:
        AdcCh32vRegularChannel(ADC_TypeDef* adc, u16 pin) : _adc(adc) {

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

            GPIO_InitStructure.GPIO_Pin = pin;
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

            ADC_RegularChannelConfig(_adc, getChanFromGpio(pin), 1, ADC_SampleTime_239Cycles5);

            ADC_Cmd(_adc, ENABLE);

            ADC_BufferCmd(_adc, DISABLE); //disable buffer
            ADC_ResetCalibration(_adc);
            while(ADC_GetResetCalibrationStatus(_adc));
            ADC_StartCalibration(_adc);
            while(ADC_GetCalibrationStatus(_adc));
            _calibrattionValue = Get_CalibrationValue(_adc);
            //ADC_Cmd(_adc, DISABLE);
        }
        
        u16 getRawValue() {
            u16 value;
            ADC_SoftwareStartConvCmd(_adc, ENABLE);
            while(!ADC_GetFlagStatus(_adc, ADC_FLAG_EOC));
            value = ADC_GetConversionValue(_adc);
            return getConversionValue(value);
        }

        template<int TopResistor = 10000, int BottomResistor = 10000>
        float getVoltageMv() {
            u16 rawValue = getRawValue();
            constexpr float k = static_cast<float>(TopResistor + BottomResistor) / BottomResistor;
            float value = (static_cast<float>(rawValue) / _bitDepth) * _vdd * k;
            return value;
        }

        void deInit() {
            disable();
            ADC_DeInit(_adc);
        }

        ADC_TypeDef* getAdcPtr() {
            return _adc;
        }

        void enable() {
            ADC_Cmd(_adc, ENABLE);
        }

        void disable() {
            ADC_Cmd(_adc, DISABLE);
        }

        uint8_t getChanFromGpio(u16 pin) {
            if(pin == GPIO_Pin_0) {
                return 0;
            }
            if(pin == GPIO_Pin_1) {
                return 1;
            }
            if(pin == GPIO_Pin_2) {
                return 2;
            }
            if(pin == GPIO_Pin_3) {
                return 3;
            }
            if(pin == GPIO_Pin_4) {
                return 4;
            }
            if(pin == GPIO_Pin_5) {
                return 5;
            }
            if(pin == GPIO_Pin_6) {
                return 6;
            }
            if(pin == GPIO_Pin_7) {
                return 7;
            }
            return 0;
        }

        u16 getConversionValue(s16 val) {
            if((val + _calibrattionValue) < 0)
                return 0;
            if((_calibrattionValue + val) > 4095||val==4095)
                return 4095;
            return (val + _calibrattionValue);
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

        u16 _vdd = 3300;
        u16 _bitDepth = 4095;
        ADC_TypeDef* _adc;
        s16 _calibrattionValue = 0;
    };
}
