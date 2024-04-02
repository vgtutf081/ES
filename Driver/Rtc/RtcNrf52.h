#pragma once

#include "nrf_drv_rtc.h"
#include <stdint.h>
#include "nrf_drv_clock.h"
#include "GpioNrf52.h"

namespace ES {

    struct Time {
    public:
        uint16_t sec;
        uint16_t min;
        uint16_t hour;
        uint16_t day;
        uint16_t month;
        uint16_t year;
        uint16_t dow;
    };

    class RtcNrf52 {
    public:
        static constexpr float timerFreq = 32768.f;

        enum class TickFreqRtc : uint16_t {
            Hz8 = 4095,
            Hz100 = 327
        };

        constexpr RtcNrf52(nrf_drv_rtc_t rtcInstance, uint8_t compareChannel, TickFreqRtc tickFreqHz = TickFreqRtc::Hz8) : _rtc(rtcInstance), _tickFreqHz(uint16_t(tickFreqHz)) {
            nrf_drv_clock_lfclk_request(NULL);

            uint32_t err_code = 0;

            //Initialize RTC instance
            nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
            config.prescaler = timerFreq / _tickFreqHz - 1; //max prescaler = 4095
            err_code = nrf_drv_rtc_init(&_rtc, &config, rtcEventHandlerStatic, this);
            APP_ERROR_CHECK(err_code);

            //Enable tick event & interrupt
            nrf_drv_rtc_tick_enable(&_rtc, true);

            nrf_drv_rtc_overflow_enable(&_rtc, true);

            //Set compare channel to trigger interrupt after COMPARE_COUNTERTIME seconds
            err_code = nrf_drv_rtc_cc_set(&_rtc, compareChannel, 0, true);
            APP_ERROR_CHECK(err_code);

            //Power on RTC instance
            nrf_drv_rtc_enable(&_rtc);
        }

    protected:

        virtual void rtcEventHandler(nrf_drv_rtc_int_type_t type) {

        }

    private:

        static void rtcEventHandlerStatic(nrf_drv_rtc_int_type_t int_type, void * p_context) {
            RtcNrf52* _this = reinterpret_cast<RtcNrf52*>(p_context);
            _this->rtcEventHandler(int_type);
        }



        uint32_t _tickFreqHz;

        uint8_t c = 0;
        const nrf_drv_rtc_t _rtc;
    };

    class Clock : public RtcNrf52 {
    public:

        const uint16_t TickFreqHz = static_cast<uint16_t>(TickFreqRtc::Hz8);

        Clock(nrf_drv_rtc_t rtcInstance, uint8_t compareChannel) : RtcNrf52(rtcInstance, compareChannel) {
            _gpio.configureOutput();
        }

        void rtcEventHandler(nrf_drv_rtc_int_type_t type) final {
            subSecond++;
            if(subSecond == TickFreqHz) {
                subSecond = 0;
                seconds++;
                _gpio.toggle();
            }
        }

    private: 
        uint8_t subSecond = 0;
        uint8_t seconds = 1;

        ES::Driver::Gpio::Nrf52Gpio _gpio{FREE_PIN1};
    };
}