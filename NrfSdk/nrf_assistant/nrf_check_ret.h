#pragma once

#include "nrf_error.h"

namespace ES::CheckErrorCode {
    inline bool success(ret_code_t code) {
        if(code == NRF_SUCCESS) {
            return true;
        }   
        return false;
    }
}