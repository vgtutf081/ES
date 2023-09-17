#pragma once

#if defined (CH32V)
#include "GpioCh32v.h"
#include "GpioCh32Modes.h"
#elif defined (NRF)
#include "GpioNrfModes.h"
#include "GpioNrf52.h"
#endif