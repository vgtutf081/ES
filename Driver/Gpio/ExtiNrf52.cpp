#include "app_config.h"

#if defined(ES_NRF_GPIO)

#include "IGpio.h"
#include "ExtiCommon.h"

namespace ES::Driver::Gpio {
    static void gpioteHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action, void* context) {
        static_cast<void>(action);
        reinterpret_cast<InterruptCallback*>(context)->onGpioInterrupt(pin);
    }

    void configureInterrupt(IGpio& pin, PullMode pullMode, InterruptMode interruptMode, InterruptCallback* callback) {
        nrfx_gpiote_in_config_t gpioteConfig;
        gpioteConfig.is_watcher = false;
        gpioteConfig.hi_accuracy = true;
        gpioteConfig.pull = static_cast<nrf_gpio_pin_pull_t>(pullMode);
        gpioteConfig.sense = static_cast<nrf_gpiote_polarity_t>(interruptMode);
        gpioteConfig.skip_gpio_setup = false;
        if(nrfx_gpiote_in_init(pin.getPortAndPin(), &gpioteConfig, gpioteHandler, callback) != NRFX_SUCCESS) {
            return;
        }
        nrfx_gpiote_in_event_enable(pin.getPortAndPin(), true);
    }

    void releaseInterrupt(InterruptCallback* callback) {

    }

    void releaseInterrupt(IGpio& pin) {
        nrfx_gpiote_in_uninit(pin.getPortAndPin());
    }
}

#endif