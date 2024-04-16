#pragma once

#include "nrf_drv_rtc.h"
#include <stdint.h>
#include "nrf_drv_clock.h"
#include "GpioNrf52.h"

namespace ES {

    struct Time {
    public:
        uint16_t sec = 0;
        uint16_t min = 0;
        uint16_t hour = 0;
        uint16_t day = 0;
        uint16_t month = 0;
        uint16_t year = 0;
        uint16_t dow = 0;
    };

    class RtcNrf52 {
    public:
        static constexpr float timerFreq = 32768.f;

        enum class TickFreqRtc : uint16_t {
            Hz8 = 8,
            Hz100 = 100
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

            //nrf_drv_rtc_overflow_enable(&_rtc, true);

            //Set compare channel to trigger interrupt after COMPARE_COUNTERTIME seconds
            //err_code = nrf_drv_rtc_cc_set(&_rtc, compareChannel, 0, true);
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

        constexpr Clock(nrf_drv_rtc_t rtcInstance, uint8_t compareChannel) : RtcNrf52(rtcInstance, compareChannel) {
            //_gpio.configureOutput();
        }

        void setTime(unsigned int hour, unsigned int min, unsigned int sec, unsigned int day, unsigned int mon, unsigned int year, unsigned int dow) {
            _actualTime.hour = hour; //TODO may be -1
            _actualTime.min = min;
            _actualTime.sec = sec;
            _actualTime.day = day;
            _actualTime.month = mon;
            _actualTime.year = year;
            _actualTime.dow = dow;
        }

    private:

        void rtcEventHandler(nrf_drv_rtc_int_type_t type) final {
            _subSecond++;
            if(_subSecond == TickFreqHz) {
                _subSecond = 0;
                _actualTime.sec++;
                //_gpio.toggle();
            }
            if(_actualTime.sec == 60) {
                _actualTime.min++;
                _actualTime.sec = 0;
            }
            if(_actualTime.min == 60) {
                _actualTime.hour++;
                _actualTime.hour = 0;
            }
            if(_actualTime.hour == 24) {
                _actualTime.day++;
                _actualTime.dow++;
                _actualTime.hour = 0;
            }
            if(_actualTime.dow = 7) {
                _actualTime.dow = 0;
            }
            if(_actualTime.day == 28) {
                if(_actualTime.month == 2 - 1) {
                    if((_actualTime.year - 1) % 4 != 0) {
                        _actualTime.month++;
                        _actualTime.day = 0;
                    }
                }
            }
            if(_actualTime.day == 29) {
                if(_actualTime.month == 2 - 1) {
                    if((_actualTime.year - 1) % 4 == 0) {
                        _actualTime.month++;
                        _actualTime.day = 0;
                    }
                }
            }
            if(_actualTime.day == 30) {
                if(_actualTime.month == 4 - 1 || _actualTime.month == 6 - 1 || _actualTime.month == 9 - 1 || _actualTime.month == 11 - 1) {
                    _actualTime.month++;
                    _actualTime.day = 0;
                }
            }
            if(_actualTime.day == 31) {
                _actualTime.month++;
                _actualTime.day = 0;
            }
            if(_actualTime.month == 12) {
                _actualTime.year++;
            }
        }

    protected:

        Time _actualTime{};

    private: 

        uint8_t _subSecond = 0;
        //ES::Driver::Gpio::Nrf52Gpio _gpio{FREE_PIN1};
    };
}