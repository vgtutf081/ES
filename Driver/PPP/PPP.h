#pragma once

#include "task.h"
#include "CommonTools.h"

#include <cstdint>
#include <functional>

#define ALIGN_4  __align(4)

namespace ES::Driver {

    typedef union UipIpaddrt {
        uint8_t  u8[4];			/* Initializer, must come first!!! */
        uint16_t u16[2];
        uint32_t u32;
    } uip_ip4addr_t;

    struct UipFwNetif {
    struct UipFwNetif *next;  /**< Pointer to the next interface when
                    linked in a list. */
    UipIpaddrt ipAddr;            /**< The IP address of this interface. */
    UipIpaddrt netMask;           /**< The netmask of the interface. */
    uint8_t (* output)(void);
                                /**< A pointer to the function that
                   sends a packet. */
    };

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

    static constexpr uint8_t IpcpAddress = 0x03;

    static constexpr uint16_t IpcpTimeut = 1000;

    static constexpr uint16_t IpcpIpAddress = 0x03;
    static constexpr uint16_t IpcpPrimaryDns = 0x81;
    static constexpr uint16_t IpcpSecondaryDns = 0x83;

    static constexpr uint8_t IpcpRetryCount = 6;

    class LCP {
    public:

        uint8_t lcpList[7] = {
            LpcMagicNumber,
            LpcPfc,
            LpcAcfc,
            LpcAuth,
            LpcAccm ,
            LpcMru,
            0
        };

        struct lcpPacket {
            uint8_t code;
            uint8_t id;
            uint16_t len;
            uint8_t data[8];
        };

        enum class LcpState : uint16_t {
            Idle = 0,
            LcpTxUp = 0x01,
            LcpRxUp = 0x02,
            LcpRxAuth = 0x010,
            LcpTermPeer = 0x20,
            LcpRxTimeout = 0x40,
            LcpTxTimeout = 0x80
        };

        LCP(const std::function<void(size_t)>& callbackBadPacket) : _callbackSendPacket(callbackBadPacket) {

        }
        
        void lcpInit() {
            _lcpRetry = 0;
            _state = static_cast<uint16_t>(LcpState::Idle);
        }

        void lcpTask(uint8_t *buffer, size_t &t)  {
            //if(!(_state & static_cast<uint16_t>(LcpState::LcpTxUp)) && !(_state & static_cast<uint16_t>(LcpState::LcpTxTimeout))) {
    /* Check if we have a request pending */
                //if(1 == lpcTimerTimeout()) {
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
                //}
            //}
        }

        void lcpRecieve(uint8_t* buffer, uint16_t count, uint8_t &pppId) {
            _bptr = buffer;
            uint8_t *tptr;
            uint8_t error = 0;
            uint16_t len, j;
            uint8_t packetSize;
            switch(*_bptr++) {
            case ConfReq:
                _bptr++;
                len = (*_bptr++ << 8);
                len |= *_bptr++;
                packetSize = scanPacket(buffer, _bptr, (uint16_t)(len-4));
                if(packetSize) {
                    _callbackSendPacket(packetSize);
                }
                else {
                          /* lets try to implement what peer wants */
                    tptr = _bptr = buffer;
                    _bptr += 4;			/* skip code, id, len */
                    error = 0;
                          /* first scan for unknown values */
                    while(_bptr < buffer+len) {
                        switch(*_bptr++) {
                        case LpcMru:	/* mru */
                            j = *_bptr++;
                            j -= 2;
                            if(j == 2) {
                                _pppTxMru = ((int)*_bptr++) << 8;
                                _pppTxMru |= *_bptr++;
                            }
                            else {
                                //not ok
                            }
                        break;
                        case LpcAccm:	/*		*/
                            _bptr++;		/* skip length */	
                            j = *_bptr++;
                            j += *_bptr++;
                            j += *_bptr++;
                            j += *_bptr++;
                            if(j == 0) {
                                // ok
                            } else if(j!=0) {
                                // ok
                            } else {
                                /*
                                * fail we only support default or all zeros
                                */
                                error = 1;
                                *tptr++ = LpcAccm;
                                *tptr++ = 0x6;
                                *tptr++ = 0;
                                *tptr++ = 0;
                                *tptr++ = 0;
                                *tptr++ = 0;
                            }
                        break;
                        case LpcAuth:
                            _bptr++;
                            if((*_bptr++ == 0xc0) && (*_bptr++ == 0x23)) {
                                /* negotiate PAP */

                                _state |= static_cast<uint16_t>(LcpState::LcpRxAuth);;	

                            } else {
                                /* we only support PAP */
                                error = 1;
                                *tptr++ = LpcAuth;
                                *tptr++ = 0x4;
                                *tptr++ = 0xc0;
                                *tptr++ = 0x23;
                            }
                        break;
                        case LpcMagicNumber:
                            /*
                            * Compair incoming number to our number (not implemented)
                            */
                            _bptr++;		/* for now just dump */
                            _bptr++;
                            _bptr++;
                            _bptr++;
                            _bptr++;
                            break;
                            case LpcPfc:
                            _bptr++;
                            /*tflag|=PPP_PFC;*/
                            break;
                            case LpcAcfc:
                            _bptr++;
                            /*tflag|=PPP_ACFC;*/
                            break;
        
                        }
                        if(error) {
                        /* write the config NAK packet we've built above, take on the header */
                            _bptr = buffer;
                            *_bptr++ = ConfNak;		/* Write Conf_rej */
                            *_bptr++;/*tptr++;*/		/* skip over ID */

                            /* Write new length */
                            *_bptr++ = 0;
                            *_bptr = tptr - buffer;

                            /* write the reject frame */
                            // Send packet ahdlc_txz(procol,header,data,headerlen,datalen);				
                            _callbackSendPacket(static_cast<uint16_t>(tptr-buffer));                            
                        } 
                        else {
                            /*
                            * If we get here then we are OK, lets send an ACK and tell the rest
                            * of our modules our negotiated config.
                            */
                            _bptr = buffer;
                            *_bptr++ = ConfAck;		/* Write Conf_ACK */
                            _bptr++;				/* Skip ID (send same one) */
                            /*
                            * Set stuff
                            */
        
                            /* write the ACK frame */
                            /* Send packet ahdlc_txz(procol,header,data,headerlen,datalen);	*/
                            _callbackSendPacket(count);
                            _state |= static_cast<uint16_t>(LcpState::LcpRxUp);
                            /* expire the timer to make things happen after a state change */
                            /*timer_expire();*/
                        
                        }
                    }
                    break;    
                case ConfAck:			/* config Ack   Anytime we do an ack reset the timer to force send. */
                        /* check that ID matches one sent */
                    if(*_bptr++ == pppId) {	
                        /* Change state to PPP up. */
                        /* copy negotiated values over */
                        
                        _state |= static_cast<uint16_t>(LcpState::LcpTxUp);		
                        
                        /* expire the timer to make things happen after a state change */
                        lcpTimerExpire();
                    }
                    else {
                        //bad
                    }
                    break;
                case ConfNak:			/* Config Nack */
                    pppId++;
                    break;
                case ConfRej:			/* Config Reject */
                    pppId++;
                    break;
                case TermReq:			/* Terminate Request */
                    _bptr = buffer;
                    *_bptr++ = TermAck;			/* Write TERM_ACK */
                    /* write the reject frame */
                    /* Send packet ahdlc_txz(procol,header,data,headerlen,datalen); */
                    _callbackSendPacket(count);
                    _state &= ~static_cast<uint16_t>(LcpState::LcpTxUp);		
                    _state |= static_cast<uint16_t>(LcpState::LcpTermPeer);	
                    break;
                case TermAck:
                    break;
                default:
                    break;         
                }
            }
        }

        uint16_t scanPacket(uint8_t *buffer, uint8_t *options, uint16_t len) {
            uint8_t *tlist;
            uint8_t *tptr;
            uint8_t bad = 0;
            uint8_t i, j, good;

            _bptr = tptr = options;
            /* scan through the packet and see if it has any unsupported codes */
            while(_bptr < options + len) {
                /* get code and see if it matches somwhere in the list, if not
                we don't support it */
                i = *_bptr++;
                
                tlist = lcpList;
                good = 0;
                while(*tlist) {
                    if(i == *tlist++) {
                        good = 1;
                    break;
                    }
                }
                if(!good) {
                /* we don't understand it, write it back */
                    bad = 1;
                    *tptr++ = i;
                    j = *tptr++ = *_bptr++;
                    for(i = 0; i < j - 2; ++i) {
                        *tptr++ = *_bptr++;
                    }
                } else {
                /* advance over to next option */
                    _bptr += *_bptr - 1;
                }
            }
            
            /* Bad? if we we need to send a config Reject */
            if(bad) {
                return makeBadPacket(tptr, buffer);
            }
            else {
                return 0;
            }
        }

        uint16_t makeBadPacket(uint8_t *tptr, unsigned char *buffer) {
            /* write the config Rej packet we've built above, take on the header */
            _bptr = buffer;
            *_bptr++ = ConfRej;		/* Write Conf_rej */
            _bptr++;			/* skip over ID */
            *_bptr++ = 0;
            *_bptr = tptr - buffer;
            /* length right here? */
            
            return static_cast<uint16_t>(tptr - buffer);
        }

        void lcpTimerExpire()
        {
            _lpcTime = xTaskGetTickCount() - LpcTimeout;
        }

        bool lpcTimerTimeout() {
            return ((xTaskGetTickCount() - _lpcTime) > LpcTimeout);
        }

        void lpcTimerSet()
        {
            _lpcTime = xTaskGetTickCount();
        }

        private:
        uint16_t _pppTxMru = 0;
        std::function<void(size_t)> _callbackSendPacket;
        uint8_t *_bptr;
        lcpPacket *_pkt;
        uint32_t _lpcTime = 0;
        uint8_t _lcpRetry = 0;
        uint16_t _state = static_cast<uint16_t>(LcpState::Idle);

    };

    class IPCP {
    public:

        enum class IpcpState : uint16_t {
            Idle = 0,
            IpcpTxUp = 0x01,
            IpcpRxUp = 0x02,
            IpcpIpBit = 0x04,
            IpcpTxTimeout = 0x08,
            IpcpPriDnsBit = 0x08,
            IpcpSecDnsBIt = 0x10
        };

        typedef struct IpcpPacket
        {
            uint8_t code;
            uint8_t id;
            uint16_t len;
            uint8_t data[1];	//dim2dim
        } ipcpPacket;

        IPCP (const std::function<void(size_t)>& callbackBadPacket) : _callbackSendPacket(callbackBadPacket) {

        }

        uint8_t ipcpList[2] = {0x3, 0};	

        void ipcp_task(uint8_t *buffer) {
            uint16_t	t;
            ipcpPacket *pkt;
  
  // IPCP tx not up and hasn't timed out then lets see if we need to send a request
  if(!(_state & static_cast<uint16_t>(IpcpState::IpcpTxUp)) && !(_state & static_cast<uint16_t>(IpcpState::IpcpTxTimeout))) 
  {
    // Have we timed out? (combide the timers?)
    if(_ipcpRetry > IpcpRetryCount)
    {
    	_state |= static_cast<uint16_t>(IpcpState::IpcpTxTimeout);	
      ppp_reconnect();
      return;
    }
  
    // Check if we have a request pending
    if(IPCP_TIMER_timeout(IPCP_TIMEOUT * (ipcp_retry+1) * (ipcp_retry+1))) 
    {
      // No pending request, lets build one
      pkt=(IPCPPKT *)buffer;		
      
      // Configure-Request only here, write id
      pkt->code = CONF_REQ;
      pkt->id = ppp_id;
      bptr = pkt->data;       

      // Write options, we want IP address, and DNS addresses if set.
      // Write zeros for IP address the first time 
      *bptr++ = IPCP_IPADDRESS;
      *bptr++ = 0x6;
      *bptr++ = pppif.ipaddr.u8[0];
      *bptr++ = pppif.ipaddr.u8[1];
      *bptr++ = pppif.ipaddr.u8[2];
      *bptr++ = pppif.ipaddr.u8[3];

     #ifdef IPCP_GET_PRI_DNS
      if(!(ipcp_state & IPCP_PRI_DNS_BIT)) 
      {
	      // Write zeros for IP address the first time
	      *bptr++ = IPCP_PRIMARY_DNS;
	      *bptr++ = 0x6;
	      *bptr++ = ((u8_t*)pri_dns_addr)[0];
	      *bptr++ = ((u8_t*)pri_dns_addr)[1];
	      *bptr++ = ((u8_t*)pri_dns_addr)[2];
	      *bptr++ = ((u8_t*)pri_dns_addr)[3];
      }
     #endif
     
     #ifdef IPCP_GET_SEC_DNS
      if(!(ipcp_state & IPCP_SEC_DNS_BIT)) 
      {
	      // Write zeros for IP address the first time
	      *bptr++ = IPCP_SECONDARY_DNS;
	      *bptr++ = 0x6;
	      *bptr++ = ((u8_t*)sec_dns_addr)[0];
	      *bptr++ = ((u8_t*)sec_dns_addr)[1];
	      *bptr++ = ((u8_t*)sec_dns_addr)[2];
	      *bptr++ = ((u8_t*)sec_dns_addr)[3];
      }
     #endif
      // Write length
      t = bptr - buffer;
      // length here -  code and ID + 
      pkt->len = htons(t);	
      
      LOG_PPP(8,"**Sending IPCP Request packet");
      
      // Send packet ahdlc_txz(procol,header,data,headerlen,datalen);
      ahdlc_tx(IPCP, 0, buffer, 0, t);

      // Set timer
      IPCP_TIMER_set();
      // Inc retry
      ipcp_retry++;
    }
  }
}
    
        void ipcpRecieve(uint8_t* buffer, uint16_t count, uint8_t &pppId) {
            _bptr = buffer;
            uint16_t len;
            size_t packetSize;
	
            switch(*_bptr++) {
            case ConfReq:
      // parce request and see if we can ACK it
                ++_bptr;
                len = (*_bptr++ << 8);
                len |= *_bptr++;
                packetSize = scanPacket(buffer, _bptr, (uint16_t)(len - 4));
                if(packetSize) {
                    _callbackSendPacket(packetSize);
                }
                else {
                    if(IpcpAddress == *_bptr++) {
	        // dump length
	                    ++_bptr;
	                    _bptr += 4;
                    }
                    else 
                    {
                        //bad
                    }
                // If we get here then we are OK, lets send an ACK and tell the rest
                // of our modules our negotiated config.
                
                _state |= static_cast<uint16_t>(IpcpState::IpcpRxUp);
                _bptr = buffer;
                *_bptr++ = ConfAck;		// Write Conf_ACK 
                _bptr++;				        // Skip ID (send same one) 
        
        //Set stuff
	
        // write the ACK frame
                _callbackSendPacket(count);
                }
                break;
            case ConfAck:			// config Ack
      
                // Parse out the results
                // Dump the ID and get the length.
                
                // dump the ID
                _bptr++;

                // get the length
                len = (*_bptr++ << 8);
                len |= *_bptr++;
        
                _state |= static_cast<uint16_t>(IpcpState::IpcpTxUp);
		
      // expire the timer to make things happen after a state change
                ipcpTimerExpire();
                break;
            case ConfNak:			// Config Nack
      // dump the ID 
                _bptr++;
      // get the length 
                len = (*_bptr++ << 8);
                len |= *_bptr++;

      // Parse ACK and set data
                while(_bptr < buffer + len) 
                {
                    switch(*_bptr++) 
                    {
                        case IpcpIpAddress:
	                    // dump length
                        _bptr++;
                        _pppIf.ipAddr.u8[0] = *_bptr++;
                        _pppIf.ipAddr.u8[1] = *_bptr++;
                        _pppIf.ipAddr.u8[2] = *_bptr++;
                        _pppIf.ipAddr.u8[3] = *_bptr++;
                        uipFwRegister(&_pppIf);
                        break;
                    default:
                        asm("nop");//problem
                    }
                }
                pppId++;
                /* expire the timer to make things happen after a state change */
                ipcpTimerExpire();
                break;
                case ConfRej:			/* Config Reject */
                /* Remove the offending options*/
                pppId++;
                /* dump the ID */
                _bptr++;
                /* get the length */
                len = (*_bptr++ << 8);
                len |= *_bptr++;

                /* Parse ACK and set data */
                while(_bptr < buffer + len) {
                    switch(*_bptr++) {
                        case IpcpIpAddress:
                        _state |= static_cast<uint16_t>(IpcpState::IpcpIpBit);
                        _bptr += 5;
                        break;
                        default:
                        asm("nop"); //unknown
                    }
                }
                /* expire the timer to make things happen after a state change */
                /*timer_expire(); */
                break;
                default:
                asm("nop");//unknown
                }
            }

        uint16_t scanPacket(uint8_t *buffer, uint8_t *options, uint16_t len) {
            uint8_t *tlist;
            uint8_t *tptr;
            uint8_t bad = 0;
            uint8_t i, j, good;

            _bptr = tptr = options;
            /* scan through the packet and see if it has any unsupported codes */
            while(_bptr < options + len) {
                /* get code and see if it matches somwhere in the list, if not
                we don't support it */
                i = *_bptr++;
                
                tlist = ipcpList;
                good = 0;
                while(*tlist) {
                    if(i == *tlist++) {
                        good = 1;
                    break;
                    }
                }
                if(!good) {
                /* we don't understand it, write it back */
                    bad = 1;
                    *tptr++ = i;
                    j = *tptr++ = *_bptr++;
                    for(i = 0; i < j - 2; ++i) {
                        *tptr++ = *_bptr++;
                    }
                } else {
                /* advance over to next option */
                    _bptr += *_bptr - 1;
                }
            }
            
            /* Bad? if we we need to send a config Reject */
            if(bad) {
                return makeBadPacket(tptr, buffer);
            }
            else {
                return 0;
            }
        }

        uint16_t makeBadPacket(uint8_t *tptr, unsigned char *buffer) {
            /* write the config Rej packet we've built above, take on the header */
            _bptr = buffer;
            *_bptr++ = ConfRej;		/* Write Conf_rej */
            _bptr++;			/* skip over ID */
            *_bptr++ = 0;
            *_bptr = tptr - buffer;
            /* length right here? */
            
            return static_cast<uint16_t>(tptr - buffer);
        }

        
        void ipcpTimerExpire()
        {
            _ipcpTime = xTaskGetTickCount() - (IpcpTimeut * (_ipcpRetry+1) * (_ipcpRetry+1));
        }

        void uipFwRegister(struct UipFwNetif *netif){
            netif->next = _netifs;
            _netifs = netif;
        }

    private:

        struct UipFwNetif *_netifs = NULL;
        struct UipFwNetif _pppIf{};
        uint8_t _ipcpRetry = 0;
        uint16_t _ipcpTime = 0;
        uint16_t _state = static_cast<uint16_t>(IpcpState::Idle);
        std::function<void(size_t)> _callbackSendPacket;
        uint8_t *_bptr;
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

        PPP(const std::function<void(size_t)>& pppSend) : _pppSend(pppSend) {
        }

        unsigned char* getLcpPacket(size_t &packetLength) {
            size_t dataLen = 0;
            _lcp.lcpTask(_buffer, dataLen);
            if (AhdlcTxOffline && (_ahdlcTxOffline++ > AhdlcTxOffline)) {
                _ahdlcTxOffline = 0;
                //ppp_reconnect();
                return 0;
            }
            prepareNewLcpPacket();

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

        void sendLcp(size_t size) {
            prepareNewLcpPacket();
            for(size_t i = 0; i < size; ++i) {    
                putCharToLcpPackage(_buffer[i]);
            }
            uint16_t i = _ahdlcTxCrc ^ 0xFFFF;
            putCharToLcpPackage((uint8_t)(i & 0xFF));
            putCharToLcpPackage((uint8_t)((i >> 8) & 0xFF));
            putChar(LcpFlag);
            _pppSend(_n);
        }

        void sendIpcp(size_t size) {
            _pppSend(_n);
        }

        void prepareNewLcpPacket() {
            prepareNewPacket();
            putChar(LcpFlag);
            putCharToLcpPackage(Address);
            putCharToLcpPackage(Control);
            putCharToLcpPackage(static_cast<uint8_t>(LcpProtocol >> 8));
            putCharToLcpPackage(static_cast<uint8_t>(LcpProtocol & 0xff));
        }

        void prepareNewPacket() {
            _n = 0;
            _ahdlcTxCrc = ahdlcCrcInitialValue;
        }

    private: 

        void putCharToLcpPackage(unsigned char c) {
            crcAdd(c);

            if  ((c == 0x7d) || (c == 0x7e) || 
                ((c < 0x20)/*||
                (_ahdlcFlags & static_cast<uint8_t>(AhdlcFlag::PppTxAsyncMap)) == 0*/)) {
                /* send escape char and xor byte by 0x20 */
                putChar(0x7d);
                c ^= 0x20;
            }
            putChar(c);
        }

        void putChar(unsigned char c) {
            _bufferChar[_n] = c;
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

        uint8_t _pppId = 0;
        std::function<void(size_t)> _lcpCallback = std::bind(&PPP::sendLcp, this, std::placeholders::_1);
        std::function<void(size_t)> _ipcpCallback = std::bind(&PPP::sendIpcp, this, std::placeholders::_1);
        std::function<void(size_t)> _pppSend;
        size_t _n = 0;
        uint8_t _ahdlcFlags = static_cast<uint8_t>(AhdlcFlag::None);
        uint16_t _ahdlcTxCrc = 0;
        uint8_t _ahdlcTxOffline = 0;
        uint8_t _buffer[32];
        unsigned char _bufferChar[128];
        LCP _lcp{_lcpCallback};
        LCP _ipcp{_ipcpCallback};
    };

}

