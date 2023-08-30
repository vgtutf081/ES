#pragma once

#include "nrf_drv_timer.h"

namespace ES::Driver::Timer {
    class TimerNrf52 {
    public:
        TimerNrf52(nrfx_timer_t instance) : _instance(instance) {

        }

        nrfx_err_t init(uint32_t timeMs, nrfx_timer_event_handler_t timerEventHandler, void * context) {
            nrfx_timer_config_t config;
            config.frequency = NRF_TIMER_FREQ_1MHz;
            config.mode = NRF_TIMER_MODE_TIMER;
            config.bit_width = NRF_TIMER_BIT_WIDTH_32;
            config.interrupt_priority = APP_IRQ_PRIORITY_LOWEST;
            config.p_context = context;

            uint32_t timeTicks = 0;

            nrfx_err_t code = nrf_drv_timer_init(&_instance, &config, timerEventHandler);

            timeTicks = nrf_drv_timer_ms_to_ticks(&_instance, timeMs);
            nrf_drv_timer_extended_compare(
                &_instance, NRF_TIMER_CC_CHANNEL0, timeTicks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

            return code;
        }

        void start() {
            nrf_drv_timer_enable(&_instance);
        }

        void stop() {
            nrf_drv_timer_disable(&_instance);
        }

        void clear() {
            nrf_drv_timer_clear(&_instance);
        }

        void setTimerEventCallback(void * callback) {
            _callback = callback;
        }

        uint32_t getCc() {
            return nrfx_timer_capture_get(&_instance, NRF_TIMER_CC_CHANNEL0);
        }

    private:

        void * _callback = nullptr;
        nrfx_timer_t _instance;
        uint32_t _timeMs;
    };
}
