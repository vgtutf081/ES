#pragma once 

namespace ES::Driver {
    
    enum ModuleEnableStatus {
        Disabled,
        Enabled,
        Boot,
        Failed
    };

    enum PhoneStatus {
        Idle,
        OutgoingPreCall,
        OutgoingCall,
        IncomingCall,
        IncomingPreCall
    };

    enum ModuleStatus {
        None,
        WaitingStatus,
        WaitingAtCommandRepeat,
        WaitingReadyStatus,
        WaitingForOk,
        WaitingForData
    };

    enum DataType {
        GpsData,
        CopsData,
        NoneData,
        CallData,
        MissedCallData
    };

    enum CardinalDirections {
        North,
        South,
        East,
        West
    };

    enum ATS : uint8_t {
        Gsm  = 0,
        GsmCOmpact = 1,
        Utran = 2,
        Eutran = 7,
        Cdma_Hdr = 8
    };

    enum GsmOperator {
        Tele2,
        Mts,
        Megafon,
        Beeline,
        Rostelecom,
        Undefined
    };

    union Date {
        uint8_t day;
        uint8_t month;
        uint16_t year;
    };

    union Time {
        uint8_t hours;
        uint8_t minutes;
        uint16_t secondsMs;
    };

    struct GpsData {
        public:

        GpsData() = default;

        float latitude;
        CardinalDirections northSouth;
        float longitude;
        CardinalDirections eastWest;
        Date date;
        Time time;
        float altitude;
        float speed;
        float course;
    };
}