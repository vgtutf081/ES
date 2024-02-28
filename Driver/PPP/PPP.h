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

    static constexpr uint16_t IpcpTimeout = 1000;

    static constexpr uint16_t IpcpIpAddress = 0x03;
    static constexpr uint16_t IpcpPrimaryDns = 0x81;
    static constexpr uint16_t IpcpSecondaryDns = 0x83;

    static constexpr uint8_t IpcpRetryCount = 6;
    
    static constexpr uint16_t PapTimeout = 10000;

    static constexpr uint8_t PapUserNameSize = 16;
    static constexpr uint8_t PapPasswordSize = 16;

    static constexpr uint8_t PapRetryCount = 3;

    static constexpr uint8_t AhdlcTxOffline = 20;
    static constexpr uint8_t LcpFlag = 0x7E;
    static constexpr uint8_t Address = 0xFF;
    static constexpr uint8_t Control = 0x03;
    static constexpr uint16_t LcpProtocol = 0xC021;
    static constexpr uint16_t ahdlcCrcInitialValue = 0xFFFF;

    static constexpr uint16_t UipLinkMtu = 0x03;
    static constexpr uint16_t UipLlhLen = 0xC021;
    static constexpr uint16_t UipBufSize = UipLinkMtu + UipLlhLen;

    static constexpr uint16_t PppRxBufferSize = 1024;


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
        
        void init() {
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

        

        private:

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

        uint16_t _pppTxMru = 0;
        std::function<void(size_t)> _callbackSendPacket;
        uint8_t *_bptr;
        lcpPacket *_pkt;
        uint32_t _lpcTime = 0;
        uint8_t _lcpRetry = 0;
        uint16_t _state = static_cast<uint16_t>(LcpState::Idle);

    };

    class PAP {
        public:

        enum class PapState : uint8_t {
            None = 0x00,
            PapTxUp = 0x01,
            PapRxUp = 0x02,
            PapRxAuthFail = 0x10,
            PapTxAuthFail = 0x20,
            PapRxTimeout = 0x80,
            PapTxTimeout = 0x80
        };

        typedef struct PapPkt {
            uint8_t code;
            uint8_t id;
            uint16_t len;
            uint8_t data[1];	//dim2dim
        } papPkt;

        PAP (const std::function<void(size_t)>& callbackBadPacket) : _callbackSendPacket(callbackBadPacket) {

        }
        
        void init() {
            _papRetry = 0;			/* We reuse ppp_retry */
            _state = static_cast<uint8_t>(PapState::None);
        }




        void PapTask(uint8_t* buffer, uint8_t &pppId) {

            uint16_t t;
            papPkt *pkt;

            /* If LCP is up and PAP negotiated, try to bring up PAP */
            if(!(_state & static_cast<uint8_t>(PapState::PapTxUp)) && !(_state & static_cast<uint8_t>(PapState::PapTxTimeout))) {
                /* Do we need to send a PAP auth packet?
                Check if we have a request pending*/
                if(1 == papTimerTimeout(PapTimeout)) {
                /* Check if we have a request pending */
                /* t=get_seconds()-pap_tx_time;
                if(	t > pap_timeout)
                {
                */
                /* We need to send a PAP authentication request */
      
                /* Build a PAP request packet */
                pkt = (papPkt *)buffer;		
                
                /* Configure-Request only here, write id */
                pkt->code = ConfReq;
                pkt->id = pppId;
                _bptr = pkt->data;
                
                /* Write options */
                t = strlen((const char*)_papUsername);
                /* Write peer length */
                *_bptr++ = (uint8_t)t;	
                memcpy(_bptr, _papUsername, t);
                _bptr+= t;


                t = strlen((const char*)_papPassword);
                *_bptr++ = (uint8_t)t;
                memcpy(_bptr, _papPassword, t);
                _bptr+= t;
			
            /* Write length */
                t = _bptr - buffer;
                /* length here -  code and ID +  */
                pkt->len = ES::CommonTools::htons(t);	
                
                /* Send packet */
                _callbackSendPacket(t);

                /* Set timer */
                papTimerSet();
                
                _papRetry++;

                 /* Have we failed? */
                if(_papRetry > PapRetryCount) 
                {
                    _state |= static_cast<uint8_t>(PapState::PapTxTimeout);	
                    //reconnect
                }
                }
            }
        }

        void papRecieve(uint8_t *buffer, uint16_t count){
            _bptr = buffer;
            uint8_t len;

            switch(*_bptr++) {
            case ConfReq:	
            //bad
                break;
            case ConfAck:			/* config Ack */
                /* Display message if debug */
                len = *_bptr++;
                *(_bptr + len) = 0;
                _state |= static_cast<uint8_t>(PapState::PapTxUp);	
                /* expire the timer to make things happen after a state change */
                papTimerExpire();
                break;
            case ConfNak:
                _state |= static_cast<uint8_t>(PapState::PapTxAuthFail);	
                /* display message if debug */
                len = *_bptr++;
                *(_bptr + len)=0;
                break;
                }
            }

        private:

            unsigned int papTimerTimeout(unsigned int x){
                if((xTaskGetTickCount() - _papTime) > x)
                    return 1;

                return 0;
            }

            void papTimerSet()
            { 
                _papTime = xTaskGetTickCount();
            }

            void papTimerExpire()
            {
                _papTime = xTaskGetTickCount() - PapTimeout;
            }

            uint8_t _papUsername[PapUserNameSize];
            uint8_t _papPassword[PapPasswordSize];
            uint16_t _papTime = 0;
            uint8_t *_bptr;
            std::function<void(size_t)> _callbackSendPacket;
            uint8_t _papRetry = 0;
            uint8_t _state = static_cast<uint8_t>(PapState::None);
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

        void ipcpTask(uint8_t *buffer, uint8_t &pppId) {
            uint16_t t;
            ipcpPacket *pkt;
  
  // IPCP tx not up and hasn't timed out then lets see if we need to send a request
            if(!(_state & static_cast<uint16_t>(IpcpState::IpcpTxUp)) && !(_state & static_cast<uint16_t>(IpcpState::IpcpTxTimeout))) 
            {
                // Have we timed out? (combide the timers?)
                if(_ipcpRetry > IpcpRetryCount)
                {
                    _state |= static_cast<uint16_t>(IpcpState::IpcpTxTimeout);	
                    // reconnect
                }
  
                // Check if we have a request pending
                if(ipcpTimerTimeout(IpcpTimeout * (_ipcpRetry+1) * (_ipcpRetry+1))) 
                {
                // No pending request, lets build one
                pkt=(ipcpPacket *)buffer;		
      
                // Configure-Request only here, write id
                pkt->code = ConfReq;
                pkt->id = pppId;
                _bptr = pkt->data;       

                // Write options, we want IP address, and DNS addresses if set.
                // Write zeros for IP address the first time 
                *_bptr++ = IpcpIpAddress;
                *_bptr++ = 0x6;
                *_bptr++ = _pppIf.ipAddr.u8[0];
                *_bptr++ = _pppIf.ipAddr.u8[1];
                *_bptr++ = _pppIf.ipAddr.u8[2];
                *_bptr++ = _pppIf.ipAddr.u8[3];

                // Write length
                t = _bptr - buffer;
                // length here -  code and ID + 
                pkt->len = ES::CommonTools::htons(t);
                
                // Send packet ahdlc_txz(procol,header,data,headerlen,datalen);
                _callbackSendPacket(t);

                // Set timer
                ipcpTimerSet();
                // Inc retry
                _ipcpRetry++;
                }
            }
        }

        void init(void){
            _state = static_cast<uint16_t>(IpcpState::Idle);
            _ipcpRetry = 0;
            _pppIf.ipAddr.u16[0] = _pppIf.ipAddr.u16[1] = _pppIf.netMask.u16[0] = _pppIf.netMask.u16[1]= 0;
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

    private:

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

        unsigned int ipcpTimerTimeout(unsigned int x){
            if((xTaskGetTickCount() - _ipcpTime) > x)
                return 1;

            return 0;
        }
        
        void ipcpTimerExpire()
        {
            _ipcpTime = xTaskGetTickCount() - (IpcpTimeout * (_ipcpRetry+1) * (_ipcpRetry+1));
        }

        void ipcpTimerSet()
        {
            _ipcpTime= xTaskGetTickCount();
        }


        void uipFwRegister(struct UipFwNetif *netif){
            netif->next = _netifs;
            _netifs = netif;
        }

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

        enum class AhdlcFlag : uint8_t {
            None = 0,
            PppEscaped = 0x01,
            PppRxReady = 0x02,
            PppRxAsyncMap = 0x08,
            PppTxAsyncMap = 0x08,
            PppPfc = 0x10,
            PppAcfc = 0x20
        };

        typedef __packed union {
            uint32_t u32[(UipBufSize + 3) / 4];
            uint8_t u8[UipBufSize];
        } uipBuft;

        PPP(const std::function<void(size_t)>& pppSend) : _pppSend(pppSend) {
        }

        enum class PppFlags : uint8_t {
            None = 0x00,
            PppEscaped = 0x01,
            PppRxReady = 0x02,
            PppRxAsyncMap = 0x8,
            PppTxAsyncMap = 0x8,
            PppPfc = 0x10,
            PppAcfc = 0x20,
        };

        enum class PppProtocols : uint16_t {
            Lcp = 0xc021,
            Pap = 0xc023,
            Ipcp = 0x8021,
            Ipv4 = 0x0021
        };

        enum class AhdlcBits : uint8_t {
            None = 0x0,
            AhdlcEscaped = 0x1,
            AhdlcRxReady = 0x2,
            AhdlcRxAsyncMap = 0x4,
            AhdlcTxAsyncMap = 0x8,
            AhdlcPfc = 0x10,
            AhdlcAcfc = 0x20
        };

        void pppInit(void)
        {
            _pppFlags = static_cast<uint8_t>(PppFlags::None);
            _pap.init();
            _ipcp.init();
            _lcp.init();
            _pppFlags = static_cast<uint8_t>(PppFlags::None);
                
            ahdlcInit(_pppRxBuffer, PppRxBufferSize); 
            ahdlcRxReady();
        }

        void ahdlcInit(uint8_t *buffer, uint16_t maxrxbuffersize) {
            _ahdlcFlags = 0 | static_cast<uint8_t>(AhdlcBits::AhdlcRxAsyncMap);
            _ahdlcRxBuffer = buffer;
            ahdlc_max_rx_buffer_size = maxrxbuffersize;
        }

        void ahdlcRxReady() {
            _ahdlcRxCount = 0;
            _ahdlcRxCrc = 0xffff;
            _ahdlcFlags |= static_cast<uint8_t>(AhdlcBits::AhdlcRxReady);
        }

        void pppConnect() {
            _pap.init();
            _ipcp.init();
            _lcp.init();
  
        /* Enable PPP */
            _pppFlags = static_cast<uint8_t>(PppFlags::PppRxReady);
        }

        void pppUpcall(uint16_t protocol, uint8_t *buffer, uint16_t len){

            /* check to see if we have a packet waiting to be processed */
            if(_pppFlags & static_cast<uint8_t>(PppFlags::PppRxReady)) {	
                /* demux on protocol field */
                PppProtocols pppProtocol = static_cast<PppProtocols>(protocol);
                switch(pppProtocol) {
                case PppProtocols::Lcp:	/* We must support some level of LCP */
                    _lcp.lcpRecieve(buffer, len, _pppId);
                    break;
                case PppProtocols::Pap:	/* PAP should be compile in optional */
                    _pap.papRecieve(buffer, len);
                    break;
                case PppProtocols::Ipcp:	/* IPCP should be compile in optional. */
                    _ipcp.ipcpRecieve(buffer, len, _pppId);
                    break;
                case PppProtocols::Ipv4:	/* We must support IPV4 */
                    memcpy(&_uipAlignedBuf.u8[UipLlhLen], buffer, len);
                    _uipLen = len;
                    break;
                default:
                    pppRejectProtocol(protocol, len);
                break;
                }
            }
        }

        void pppRejectProtocol(uint16_t protocol, uint16_t count) {
            uint16_t	i;
            uint8_t *dptr, *sptr;
            LCP::lcpPacket *pkt;
                
            /* first copy rejected packet back, start from end and work forward,
                +++ Pay attention to buffer managment when updated. Assumes fixed
                PPP blocks. */
            if((count + 6) > PppRxBufferSize) {
                /* This is a fatal error +++ do somthing about it. */
                return;
            }
            dptr = _bufferChar + count + 6;
            sptr = _bufferChar + count;
            for(i = 0; i < count; ++i) {
                *dptr-- = *sptr--;
            }

            pkt = (LCP::lcpPacket *)_bufferChar;
            pkt->code = ProtRej;		/* Write Conf_rej */
            /*pkt->id = tid++;*/			/* write tid */
            pkt->len = ES::CommonTools::htons(count + 6);
            *((uint16_t *)(&pkt->data[0])) = ES::CommonTools::htons(protocol);

            sendLcp((uint16_t)(count + 6));
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

        void sendPap(size_t size) {
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

        uint16_t ahdlc_max_rx_buffer_size;
        uint8_t* _ahdlcRxBuffer;
        uint8_t _pppRxBuffer[PppRxBufferSize];
        uint16_t _ahdlcRxCount = 0;
        uint16_t _ahdlcRxCrc = 0;
        uint8_t _ahdlcFlags = static_cast<uint8_t>(AhdlcBits::None);
        uint16_t _uipLen = 0;
        uipBuft _uipAlignedBuf{};
        uint8_t _pppFlags = static_cast<uint8_t>(PppFlags::None);
        uint8_t _pppId = 0;
        std::function<void(size_t)> _lcpCallback = std::bind(&PPP::sendLcp, this, std::placeholders::_1);
        std::function<void(size_t)> _ipcpCallback = std::bind(&PPP::sendIpcp, this, std::placeholders::_1);
        std::function<void(size_t)> _papCallback = std::bind(&PPP::sendPap, this, std::placeholders::_1);
        std::function<void(size_t)> _pppSend;
        size_t _n = 0;
        uint16_t _ahdlcTxCrc = 0;
        uint8_t _ahdlcTxOffline = 0;
        uint8_t _buffer[32];
        unsigned char _bufferChar[128];
        LCP _lcp{_lcpCallback};
        IPCP _ipcp{_ipcpCallback};
        PAP _pap{_papCallback};

    };

}

