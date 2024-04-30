#pragma once

#include <nrf.h>
#include <cstdint>
#include <cstddef>

namespace ES::Driver::Flash {
    inline void waitForReady() {
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy);
    }

    enum class FlashMode : uint32_t {
        ReadOnly = NVMC_CONFIG_WEN_Ren,
        WriteEnable = NVMC_CONFIG_WEN_Wen,
        EraseEnable = NVMC_CONFIG_WEN_Een
    };

    inline void setFlashMode(FlashMode mode) {
        NRF_NVMC->CONFIG = static_cast<uint32_t>(mode);
        __ISB();
        __DSB();
    }

    inline size_t getPageSize() {
		return NRF_FICR->CODEPAGESIZE;
    }

    inline size_t getPageCount() {
        return NRF_FICR->CODESIZE;
    }

    inline size_t erasePage(uint32_t addressFirstWord) {

        if(((addressFirstWord % getPageSize()) != 0) || (addressFirstWord / getPageSize()) >= getPageCount()) {
            return 0;
        }

        setFlashMode(FlashMode::EraseEnable);

        NRF_NVMC->ERASEPAGE = addressFirstWord;
        waitForReady();

        setFlashMode(FlashMode::ReadOnly);

        return getPageSize();
    }

    inline bool write(uint32_t address, const uint32_t* values, size_t size) {

        if(((address % sizeof(uint32_t)) != 0) || ((address + (size - 1) * sizeof(uint32_t)) >= getPageSize() * getPageCount())) {
            return false;
        }

        setFlashMode(FlashMode::WriteEnable);

        for (size_t i = 0; i < size; i++) {
            ((uint32_t*)address)[i] = values[i];
            waitForReady();
        }

        setFlashMode(FlashMode::ReadOnly);

        return true;
    }

    inline bool write(uint32_t address, const uint8_t* values, size_t size) {

        if(((address % sizeof(uint32_t)) != 0) || ((address + (size - 1) * sizeof(uint32_t)) >= getPageSize() * getPageCount())) {
            return false;
        }

        setFlashMode(FlashMode::WriteEnable);

        for (size_t i = 0; i < size; i++) {
            ((uint32_t*)address)[i] = static_cast<uint32_t>(values[i]);
            waitForReady();
        }

        setFlashMode(FlashMode::ReadOnly);

        return true;
    }

    inline uint32_t read(uint32_t address) {
        return *reinterpret_cast<const uint32_t*>(address);
    }
}