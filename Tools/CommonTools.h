#pragma once

#define ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS1(__id) ES_PP_DROP __id
#define ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS(__id)  ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS1 __id

#define ES_PP_DROP(Expression)

namespace ES::CommonTools {
    inline size_t charArraySize(const char* c) {
        size_t size = 0;
        for(size_t i = 0; c[i] != '\0'; i++) {
            size++;
        }
        return size;
    }

    inline uint16_t htons(uint16_t val)
    {
        return (uint16_t)((((uint16_t) (val)) << 8) | (((uint16_t) (val)) >> 8));
    }

    inline uint16_t htonl(uint16_t val)
    {
        return (((uint32_t)htons(val) << 16) | uint32_t((uint32_t)(val) >> 16));
    }

}