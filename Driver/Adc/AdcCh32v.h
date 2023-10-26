#pragma once

#include "IAdc.h"
#include "ch32v20x.h"
#include "ch32v20x_adc.h"
#include "ch32v20x_rcc.h"
#include "ch32v20x_gpio.h"

#include "ThreadFreeRtos.h"
#include "Bldc.h"

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

            _firstRankCh = getChanFromGpio(firstRankPin);
            _secondRankCh = getChanFromGpio(secondRankPin);
            _thirdRankCh = getChanFromGpio(thirdRankPin);

            ADC_InjectedSequencerLengthConfig(_adc, 3);
            ADC_InjectedChannelConfig(_adc, _firstRankCh, 1, ADC_SampleTime_55Cycles5);
            ADC_InjectedChannelConfig(_adc, _secondRankCh, 2, ADC_SampleTime_55Cycles5);
            ADC_InjectedChannelConfig(_adc, _thirdRankCh, 3, ADC_SampleTime_55Cycles5);

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

        void setMaxSampleTime(uint8_t channel) {
            if(channel == 0) {
                _adc->SAMPTR2 = ADC_SMP0;
            }
            if(channel == 1) {
                _adc->SAMPTR2 = ADC_SMP1;
            }
            if(channel == 2) {
                _adc->SAMPTR2 = ADC_SMP2;
            }
            if(channel == 3) {
                _adc->SAMPTR2 = ADC_SMP3;
            }
            if(channel == 4) {
                _adc->SAMPTR2 = ADC_SMP4;
            }
            if(channel == 5) {
                _adc->SAMPTR2 = ADC_SMP5;
            }
            if(channel == 6) {
                _adc->SAMPTR2 = ADC_SMP6;
            }
            if(channel == 7) {
                _adc->SAMPTR2 = ADC_SMP7;
            }
        }

        void setMaxSampleTime(MotorControl::Bldc::MotorPhase phase) {
            if(phase == MotorControl::Bldc::MotorPhase::A) {
                setMaxSampleTime(_firstRankCh);
            }
            if(phase == MotorControl::Bldc::MotorPhase::B) {
                setMaxSampleTime(_secondRankCh);
            }
            if(phase == MotorControl::Bldc::MotorPhase::C) {
                setMaxSampleTime(_thirdRankCh);
            }
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

        u8 getFirstRankCh() {
            return _firstRankCh;
        }
        u8 getSecondRankCh() {
            return _secondRankCh;
        }
        u8 getThirdRankCh() {
            return _thirdRankCh;
        }

        template<int TopResistor = 10000, int BottomResistor = 10000>
        float getVoltageMv(u16 rawValue) {
            constexpr float k = static_cast<float>(TopResistor + BottomResistor) / BottomResistor;
            float value = (static_cast<float>(rawValue) / _bitDepth) * _vdd * k;
            return value;
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

        u8 _firstRankCh;
        u8 _secondRankCh;
        u8 _thirdRankCh;

        u16 _vdd = 3300;
        u16 _bitDepth = 4095;
        ADC_TypeDef* _adc;
        s16 _calibrattionValue = 0;
    };

    class AdcCh32vRegChInjCh {
    public:
        AdcCh32vRegChInjCh(ADC_TypeDef* adc, u16 pin, u16 injPin) : _adc(adc) {

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

            GPIO_InitStructure.GPIO_Pin = injPin;
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
            ADC_InjectedChannelConfig(_adc, getChanFromGpio(injPin), 1, ADC_SampleTime_239Cycles5);
            ADC_AutoInjectedConvCmd(_adc, ENABLE);
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
            value = ADC_GetConversionValue(_adc);
            return getConversionValue(value);
        }

        u16 getRawValueInj() {
            u16 value;
            value = ADC_GetInjectedConversionValue(_adc, ADC_InjectedChannel_1);
            return getConversionValue(value);
        }

        void startConv() {
            ADC_SoftwareStartConvCmd(_adc, ENABLE);
        }

        void pollForCOnv() {
            while(!ADC_GetFlagStatus(_adc, ADC_FLAG_EOC)) {
                ES::Threading::yield();
            }
            ADC_ClearFlag(_adc, ADC_FLAG_EOC);
            while(!ADC_GetFlagStatus(_adc, ADC_FLAG_JEOC)) {
                ES::Threading::yield();
            }
            ADC_ClearFlag(_adc, ADC_FLAG_JEOC);
        }

        template<int TopResistor = 10000, int BottomResistor = 10000>
        float getVoltageMv(u16 rawValue) {
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
