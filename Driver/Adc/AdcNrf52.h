#pragma once

#include "nrfx_saadc.h"
#include "nrf_gpio.h"

#include "ThreadFreeRtos.h"

namespace ES::Driver::Adc {
    class Adc {
	public:
		Adc(nrf_saadc_resolution_t resolution = nrf_saadc_resolution_t::NRF_SAADC_RESOLUTION_10BIT) {
			nrfx_saadc_config_t config;
			config.resolution         = resolution;
			config.oversample         = NRF_SAADC_OVERSAMPLE_DISABLED;
			config.interrupt_priority = NRFX_SAADC_CONFIG_IRQ_PRIORITY;
			config.low_power_mode     = false;
			config.context = this;
			nrfx_saadc_init(&config, Adc::handler);
			channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
			channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
			channel_config.gain       = NRF_SAADC_GAIN1_5;
			channel_config.reference  = NRF_SAADC_REFERENCE_INTERNAL;
			channel_config.acq_time   = NRF_SAADC_ACQTIME_40US;
			channel_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
			channel_config.burst      = NRF_SAADC_BURST_DISABLED;
			channel_config.pin_n      = NRF_SAADC_INPUT_DISABLED;

			switch(resolution)
			{
				case nrf_saadc_resolution_t::NRF_SAADC_RESOLUTION_8BIT:
					_BitResolution = 255.f;
					break;
				case nrf_saadc_resolution_t::NRF_SAADC_RESOLUTION_10BIT:
					_BitResolution = 1023.f;
					break;
				case nrf_saadc_resolution_t::NRF_SAADC_RESOLUTION_12BIT:
					_BitResolution = 4095.f;
					break;
				case nrf_saadc_resolution_t::NRF_SAADC_RESOLUTION_14BIT:
					_BitResolution = 16383.f;
					break;
				default:
					_BitResolution = 0;
					break;
			}

		}

		~Adc() {
		    nrfx_saadc_uninit();
		}

		template<int TopResistorValue = 1, int BottomResistorValue = 1>
		float getVoltage(nrf_saadc_input_t adcPin, float vref = 3.f) {
			constexpr float k = static_cast<float>(TopResistorValue + BottomResistorValue) / BottomResistorValue;
			return getRawValue(adcPin) / _BitResolution * vref * k;
		}

        float getRawVoltage(nrf_saadc_input_t adcPin) {
            return getVoltage<0, 1>(adcPin);
        }

        void setGain(nrf_saadc_gain_t gain) {
			channel_config.gain = gain;
        }

		size_t getRawValue(nrf_saadc_input_t adcPin) {
			nrf_saadc_value_t value;
			channel_config.pin_p      = (nrf_saadc_input_t)(adcPin);
			nrfx_saadc_channel_init(0, &channel_config);
			nrfx_saadc_buffer_convert(&value, 1);
			nrfx_saadc_sample();
			while(!_ready) {
                ES::Threading::yield();
			}
			_ready = false;
			nrfx_saadc_channel_uninit(0);
			nrfx_saadc_abort();
			if(value < 0) {
				value = 0;
			}
			return static_cast<size_t>(value);
		}

	private:

		static void handler(nrfx_saadc_evt_t const * p_event, void* context) {
			auto _this = reinterpret_cast<Adc*>(context);
			if(p_event->type == NRFX_SAADC_EVT_DONE) {
				_this->_ready = true;
			}
		}

	private:
		float _BitResolution;
		nrf_saadc_channel_config_t channel_config;
		volatile bool _ready = false;
	};

    constexpr nrf_saadc_input_t toSaadcInput(uint32_t pin) {
#if defined(NRF52840_XXAA) || defined(NRF52832_XXAA) || defined(NRF52833_XXAA)
        switch(pin) {
            case NRF_GPIO_PIN_MAP(0, 2): return NRF_SAADC_INPUT_AIN0;
            case NRF_GPIO_PIN_MAP(0, 3): return NRF_SAADC_INPUT_AIN1;
            case NRF_GPIO_PIN_MAP(0, 4): return NRF_SAADC_INPUT_AIN2;
            case NRF_GPIO_PIN_MAP(0, 5): return NRF_SAADC_INPUT_AIN3;
            case NRF_GPIO_PIN_MAP(0, 28): return NRF_SAADC_INPUT_AIN4;
            case NRF_GPIO_PIN_MAP(0, 29): return NRF_SAADC_INPUT_AIN5;
            case NRF_GPIO_PIN_MAP(0, 30): return NRF_SAADC_INPUT_AIN6;
            case NRF_GPIO_PIN_MAP(0, 31): return NRF_SAADC_INPUT_AIN7;
        }
        return NRF_SAADC_INPUT_DISABLED;
    }
#elif defined(NRF52820_XXAA)
#error "NRF52820_XXAA doesn't support analogs pin"
#else
#error "Don't toSaadcInput implement for current platform"
#endif
}