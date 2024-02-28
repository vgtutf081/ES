#pragma once

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
}