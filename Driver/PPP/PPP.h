#pragma once

#include "task.h"
#include "CommonTools.h"

#include <cstdint>

namespace ES::Driver {

    class LCP {
    public:
        static constexpr uint8_t LcpRetryCount = 3;
        static constexpr uint32_t LpcTimeout = 10000;

        static constexpr uint8_t ConfReq = 0x01;
        static constexpr uint8_t ConfAck = 0x02;
        static constexpr uint8_t ConfNak = 0x03;
        static constexpr uint8_t ConfRej = 0x04;
        static constexpr uint8_t TermReq = 0x05;
        static constexpr uint8_t TermAck = 0x06;
        static constexpr uint8_t ProtRej = 0x08;

        static constexpr uint8_t LpcVenderx = 0x00;
        static constexpr uint8_t LpcMru = 0x01;
        static constexpr uint8_t LpcAccm = 0x02;
        static constexpr uint8_t LpcAuth = 0x03;
        static constexpr uint8_t LpcQuality = 0x04;
        static constexpr uint8_t LpcMagicNumber = 0x05;
        static constexpr uint8_t LpcPfc = 0x07;
        static constexpr uint8_t LpcAcfc = 0x08;

        struct lcpPacket {
            uint8_t code;
            uint8_t id;
            uint16_t len;
            uint8_t data[8];
        };

        enum class LcpState : uint16_t {
            Idle = 0,
            LcpTxUp = 0x01,
            LcpRxUpl = 0x02,
            LcpRxAuth = 0x010,
            LcpTermPeer = 0x20,
            LcpRxTimeout = 0x40,
            LcpTxTimeout = 0x80
        };
        
        void lcpInit() {
            _lcpRetry = 0;
            _state = static_cast<uint16_t>(LcpState::Idle);
        }

        void lcpTask(uint8_t *buffer, size_t &t)  {
            if(!(_state & static_cast<uint16_t>(LcpState::LcpTxUp)) && !(_state & static_cast<uint16_t>(LcpState::LcpTxTimeout))) {
    /* Check if we have a request pending */
                if(1 == lpcTimerTimeout()) {
                    _pkt = (lcpPacket *)buffer;	

                    _pkt->code = ConfReq;
                    _pkt->id = 0;
                    
                    _bptr = _pkt->data;

                    *_bptr++ = LpcAccm;
                    *_bptr++ = 0x6;
                    *_bptr++ = 0xff;
                    *_bptr++ = 0xff;
                    *_bptr++ = 0xff;
                    *_bptr++ = 0xff;

                    t = _bptr - buffer;
                    _pkt->len = ES::CommonTools::htons(t);

                    lpcTimerSet();
                    _lcpRetry++;

                    if(_lcpRetry > LcpRetryCount) 
                    {
                        _state |= static_cast<uint16_t>(LcpState::LcpTxTimeout);
                        //ppp_reconnect();
                        //TODO
                    }
                }
            }
        }

        bool lpcTimerTimeout() {
            return ((xTaskGetTickCount() - _lpcTime) > LpcTimeout);
        }

        void lpcTimerSet()
        {
            _lpcTime = xTaskGetTickCount();
        }

        private:
        
        uint8_t *_bptr;
        lcpPacket *_pkt;
        uint32_t _lpcTime = 0;
        uint8_t _lcpRetry = 0;
        uint16_t _state = static_cast<uint16_t>(LcpState::Idle);
    };

    class PPP {
    public:

        static constexpr uint8_t AhdlcTxOffline = 20;
        static constexpr uint8_t LcpFlag = 0x7E;
        static constexpr uint8_t Address = 0xFF;
        static constexpr uint8_t Control = 0x03;
        static constexpr uint16_t LcpProtocol = 0xC021;
        static constexpr uint16_t ahdlcCrcInitialValue = 0xFFFF;

        enum class AhdlcFlag : uint8_t {
            None = 0,
            PppEscaped = 0x01,
            PppRxReady = 0x02,
            PppRxAsyncMap = 0x08,
            PppTxAsyncMap = 0x08,
            PppPfc = 0x10,
            PppAcfc = 0x20
        };

        char* getLcpPacket(size_t &packetLength) {
            size_t dataLen = 0;
            _lcp.lcpTask(_buffer, dataLen);
            if (AhdlcTxOffline && (_ahdlcTxOffline++ > AhdlcTxOffline)) {
                _ahdlcTxOffline = 0;
                //ppp_reconnect();
                return 0;
            }
            size_t n = 0;
            _n = 0;
            _ahdlcTxCrc = ahdlcCrcInitialValue;
            putCharToLcpPackage(LcpFlag);
            putCharToLcpPackage(Address);
            putCharToLcpPackage(Control);

            putCharToLcpPackage(static_cast<uint8_t>(LcpProtocol >> 8));
            putCharToLcpPackage(static_cast<uint8_t>(LcpProtocol & 0xff));

            for(size_t i = 0; i < dataLen; ++i) {    
                putCharToLcpPackage(_buffer[i]);
            }

            uint16_t i = _ahdlcTxCrc ^ 0xFFFF;
            putCharToLcpPackage((uint8_t)(i & 0xFF));
            putCharToLcpPackage((uint8_t)((i >> 8) & 0xFF));
            putChar(LcpFlag);

            packetLength = _n;
            return _bufferChar;
        }

    private: 

        void putCharToLcpPackage(char c) {
            crcAdd(c);

            if  ((c == 0x7d) || (c == 0x7e) || 
                ((c < 0x20) ||
                (_ahdlcFlags & static_cast<uint8_t>(AhdlcFlag::PppTxAsyncMap)) == 0)) {
                /* send escape char and xor byte by 0x20 */
                putChar(0x7d);
                c ^= 0x20;
            }
            putChar(c);
        }

        void putChar(char c) {
            _bufferChar[_n];
            _n++;
        }

        void crcAdd(char c) {
            uint16_t b;
            uint16_t crcValue = _ahdlcTxCrc;

            b = (crcValue ^ c) & 0xFF;
            b = (b ^ (b << 4)) & 0xFF;
            b = (b << 8) ^ (b << 3) ^ (b >> 4);
            
            _ahdlcTxCrc = ((crcValue >> 8) ^ b);
        }

        size_t _n = 0;
        uint8_t _ahdlcFlags = static_cast<uint8_t>(AhdlcFlag::None);
        uint16_t _ahdlcTxCrc = 0;
        uint8_t _ahdlcTxOffline = 0;
        uint8_t _buffer[32];
        char _bufferChar[128];
        LCP _lcp{};
    };

}

