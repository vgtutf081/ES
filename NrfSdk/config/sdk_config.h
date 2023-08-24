#pragma once

#if defined (NRF52805_XXAA)
    #include "nrf52805/config/sdk_config.h"
#elif defined (NRF52810_XXAA)
    #include "nrf52810/config/sdk_config.h"
#elif defined (NRF52811_XXAA)
    #include "nrf52811/config/sdk_config.h"
#elif defined (NRF52820_XXAA)
    #include "nrf52820/config/sdk_config.h"
#elif defined (NRF52832_XXAA) || defined (NRF52832_XXAB)
    #include "nrf52832/config/sdk_config.h"
#elif defined (NRF52833_XXAA)
    #include "nrf52833/config/sdk_config.h"
#elif defined (NRF52840_XXAA)
    #include "nrf52840/config/sdk_config.h"
#elif defined (NRF5340_XXAA)
    #include "nrf5340/config/sdk_config.h"
#else
    #error "Device must be defined. See sdk config"
#endif
