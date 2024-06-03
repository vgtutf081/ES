#pragma once

#define ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS1(__id) ES_PP_DROP __id
#define ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS(__id)  ES_PP_CMSIS_UNWRAP_PERIPH_ADDRESS1 __id

#define PP_HTONS(x) ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)

//#define ALIGN_4  __align(4)

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

    
    inline uint16_t lwip_htons(uint16_t n)
    {
        return (uint16_t)PP_HTONS(n);
    }

    
    inline uint32_t lwip_htonl(uint32_t n)
    {
        return (uint32_t)PP_HTONL(n);
    }

}