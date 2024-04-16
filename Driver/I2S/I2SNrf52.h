#pragma once

#include "nrf_drv_i2s.h"
#include "I2S.h"

namespace ES::Driver {
    template<size_t PartSize>
    class I2SMasterNrf52 {
    public:

        static constexpr size_t LeftChannel = 0;
        static constexpr size_t RightChannel = 0;

		I2SMasterNrf52(uint32_t sck, uint32_t lrck, uint32_t sdout, I2S::SampleRate sampleRate, I2S::Channel channel, I2S::SampleWidth sampleWidth = I2S::SampleWidth::Bit16) : _sampleRate(sampleRate), _channel(channel), _sampleWidth(sampleWidth) { 
			nrf_drv_i2s_config_t config = NRF_DRV_I2S_DEFAULT_CONFIG;
			config.sck_pin = sck;
			config.lrck_pin = lrck;
			config.mck_pin = NRFX_I2S_PIN_NOT_USED;
			config.sdout_pin = sdout;
			config.sdin_pin = NRFX_I2S_PIN_NOT_USED;
			config.irq_priority = 6; ///< Interrupt priority.

            if(_sampleRate == I2S::SampleRate::Hz16000) {
                config.mck_setup = NRF_I2S_MCK_32MDIV21;    ///< Master clock setup.
                config.ratio = NRF_I2S_RATIO_96X;        ///< MCK/LRCK ratio.
            }
            else if(_sampleRate == I2S::SampleRate::Hz32000) {
                config.mck_setup = NRF_I2S_MCK_32MDIV21;    ///< Master clock setup.
                config.ratio = NRF_I2S_RATIO_32X;        ///< MCK/LRCK ratio.
            }
            else if(_sampleRate == I2S::SampleRate::Hz48000) {
                config.mck_setup = NRF_I2S_MCK_32MDIV21;    ///< Master clock setup.
                config.ratio = NRF_I2S_RATIO_32X;    
            }
            else if(_sampleRate == I2S::SampleRate::Hz96000) {
                config.mck_setup = NRF_I2S_MCK_32MDIV10;    ///< Master clock setup.
                config.ratio = NRF_I2S_RATIO_32X;   
            }

            if(_channel == I2S::Channel::Left) {
			    config.channels = NRF_I2S_CHANNELS_LEFT;     ///< Enabled channels.
            }
            else if(_channel == I2S::Channel::Right) {
			    config.channels = NRF_I2S_CHANNELS_RIGHT;     ///< Enabled channels.
            }
            else if(_channel == I2S::Channel::Both) {
			    config.channels = NRF_I2S_CHANNELS_STEREO;     ///< Enabled channels.
            }

            if(_sampleWidth == I2S::SampleWidth::Bit16) {
			    config.sample_width = NRF_I2S_SWIDTH_16BIT; ///< Sample width.
                config.ratio = NRF_I2S_RATIO_32X;  
                if(_sampleRate == I2S::SampleRate::Hz16000) {
                    config.mck_setup = NRF_I2S_MCK_32MDIV63;    ///< Master clock setup.
                }
                else if(_sampleRate == I2S::SampleRate::Hz32000) {
                    config.mck_setup = NRF_I2S_MCK_32MDIV30;    ///< Master clock setup.
                }
                else if(_sampleRate == I2S::SampleRate::Hz48000) {
                    config.mck_setup = NRF_I2S_MCK_32MDIV21;    ///< Master clock setup.
                }
                else if(_sampleRate == I2S::SampleRate::Hz96000) {
                    config.mck_setup = NRF_I2S_MCK_32MDIV10;    ///< Master clock setup.
                }
            }
            else if(_sampleWidth == I2S::SampleWidth::Bit24) {
			    config.sample_width = NRF_I2S_SWIDTH_24BIT; ///< Sample width.
                config.ratio = NRF_I2S_RATIO_48X;  
                if(_sampleRate == I2S::SampleRate::Hz16000) {
                    config.mck_setup = NRF_I2S_MCK_32MDIV42;    ///< Master clock setup.
                }
                else if(_sampleRate == I2S::SampleRate::Hz32000) {
                    config.mck_setup = NRF_I2S_MCK_32MDIV21;    ///< Master clock setup.
                }
                else if(_sampleRate == I2S::SampleRate::Hz48000) {
                    config.mck_setup = NRF_I2S_MCK_32MDIV15;    ///< Master clock setup.
                }
                else if(_sampleRate == I2S::SampleRate::Hz96000) {
                    config.mck_setup = NRF_I2S_MCK_32MDIV8;    ///< Master clock setup.
                }
            }
            
			config.mode = NRF_I2S_MODE_MASTER;         ///< Mode of operation.
			config.format = NRF_I2S_FORMAT_I2S;       ///< Frame format.
    		config.alignment = NRF_I2S_ALIGN_LEFT;    ///< Alignment of sample within a frame.

			auto error = nrfx_i2s_init(&config, i2sStatisDataHandler);
            NRFX_ASSERT(error != NRFX_SUCCESS)
        }

        bool addValueToSeqLeftCh(uint32_t value) {
            m_buffer_tx[0][i] = value;
            i++;
            if(i >= PartSize) {
                return 0;
            } 
            return i;
        }

        bool addValueToSeqRigthCh(uint32_t value) {
            m_buffer_tx[1][i] = value;
            i++;
            if(i >= PartSize) {
                return 0;
            } 
            return i;
        }

        void startNewSeq() {
            i = 0;
        }
        
        void start() {
             nrf_drv_i2s_buffers_t  initial_buffers;
            initial_buffers.p_tx_buffer = m_buffer_tx[0];
            initial_buffers.p_rx_buffer = nullptr;

            auto err_code = nrf_drv_i2s_start(&initial_buffers, i, 0);
            APP_ERROR_CHECK(err_code);
		}

        void stop() {
			nrfx_i2s_stop();
		}


    private:
        I2S::SampleRate _sampleRate;
        I2S::Channel _channel;
        I2S::SampleWidth _sampleWidth;
        size_t i = 0;
        uint32_t m_buffer_tx[2][PartSize];
        //uint32_t m_buffer_rx[2][PartSize];

    	static void i2sStatisDataHandler(nrfx_i2s_buffers_t const* p_released, uint32_t status) {
			
		}
    };
}