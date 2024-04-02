#pragma once

#include "SpiMasterNrf52.h"
#include "Semaphore.h"
#include <array>
#include "ThreadFreeRtos.h"

namespace ES::Driver {
	
    class W25q32fv : public ISpiEventHandler {
    public:
    	static constexpr size_t MaxSizeOfTransfer = 260;
        static constexpr size_t MemorySizeMBit = 32;

        static constexpr uint8_t StatusBusy = 0x01;
		static constexpr uint8_t StatusWel = 0x02;
		static constexpr uint8_t StatusBp0 = 0x04;
		static constexpr uint8_t StatusBp1 = 0x08;
		static constexpr uint8_t StatusBp2 = 0x10;
		static constexpr uint8_t StatusTb = 0x20;
		static constexpr uint8_t StatusSec = 0x40;
        static constexpr uint8_t StatusSrp0 = 0x80;

        static constexpr uint8_t StatusSrp = 0x01;
		static constexpr uint8_t StatusLb1 = 0x08;
		static constexpr uint8_t StatusLb2 = 0x10;
		static constexpr uint8_t StatusLb3 = 0x20;
		static constexpr uint8_t StatusCmp = 0x40;
        static constexpr uint8_t StatusSus = 0x80;

        static constexpr uint8_t StatusWps = 0x04;
		static constexpr uint8_t StatusDrv0 = 0x20;
		static constexpr uint8_t StatusDrv1 = 0x40;
        static constexpr uint8_t StatusHoldRst = 0x80;

		static constexpr size_t MaxTimeChipEraseMs = 50000;
		static constexpr size_t MaxTimeWritePageMs = 3;

		static constexpr size_t MaxAddress = 0x40000;

		using FlashBuf = std::array<uint8_t, MaxSizeOfTransfer>;

        enum class Commands : uint8_t {
            WriteEnable = 0x06,
            WriteDisable = 0x04,
            ReadStatusRegister1 = 0x05,
            WriteStatusRegister1 = 0x01,
            ReadStatusRegister2 = 0x35,
            WriteStatusRegister2 = 0x31,
            ReadStatusRegister3 = 0x15,
            WriteStatusRegister3 = 0x11,
            ChipErase = 0x60,
            ResetDevice = 0x99,
			ReadData = 0x03,
			PageProgram = 0x02
        };

		struct Cmd {
		public:
			constexpr Cmd(Commands id) : id(uint8_t(id)) {
				
			}
			uint8_t id;
		};

		struct AddressCmd : public Cmd {
		public:
			constexpr AddressCmd(Commands id, uint32_t addr) : Cmd(id),
					a0(static_cast<uint8_t>(addr >> 16)),
					a1(static_cast<uint8_t>(addr >> 8)),
					a2(static_cast<uint8_t>(addr)) {
			}
			uint8_t a0 = 0;
			uint8_t a1 = 0;
			uint8_t a2 = 0;
		};
		
        W25q32fv(const nrfx_spim_t instance, uint32_t cs, uint32_t miso, uint32_t mosi, uint32_t sck) : _spiMaster({instance, cs,  miso, mosi, sck}) {
			_spiMaster.setEventHandler(this);
        }
        
		void waitTranferEnd() {
			_transferSemaphore.take();
		}

        bool chipErase() {
			if(!tryWriteEnable()) {
				return false;
			}
			writeCmd(Commands::ChipErase);
			if(!waitWriteComplete(MaxTimeChipEraseMs)) {
				return false;
			}
			return true;
		}

		bool writePage(uint32_t address, const uint8_t* data, size_t size) {
			bool status = true;
			if(address + size > MaxAddress) {
				return false;
			}
			status &= tryWriteEnable();
			std::memcpy(&_txBuffer[sizeof(AddressCmd{Commands::PageProgram, address})], data, size);
			writeCmd(AddressCmd{Commands::PageProgram, address}, size);
			status &= waitWriteComplete(MaxTimeWritePageMs);
			auto returnData = readPage(address, size);
			if(std::memcmp(data, returnData, size) != 0) {
				return false;
			}
			return true;
		}

		FlashBuf::const_iterator readPage(uint32_t address, size_t size) {
			if(address + size <= MaxAddress) {
				waitWriteComplete();
				writeCmd(Cmd{Commands::WriteDisable});
				_txBuffer.fill(0xFF);
				return writeCmd(AddressCmd{Commands::ReadData, address}, size);
			}
			else {
				return nullptr;
			}
		}

    private:

		bool waitWriteComplete(size_t time = 10) {
			uint8_t divider = 10;
			if(time < 10) {
				divider = 1;
			}
			size_t count = 0;
			do {
				Threading::sleepForMs(time / divider);
				if(++count >= 10) {
					return false;
				}
			} while(compareRegister(StatusBusy, Cmd{Commands::ReadStatusRegister1}));
			return true;
		}

        bool tryWriteEnable() {
			size_t count = 0;
			do {
				writeCmd(Cmd{Commands::WriteEnable});
                Threading::sleepForMs(1);
				if(++count > 10) {
					return false;
				}
			} while(!compareRegister(StatusWel, Cmd{Commands::ReadStatusRegister1}));
			return true;
		}
		
		template<typename cmd>
        FlashBuf::const_iterator writeCmd(const cmd& command, size_t additionalSize = 0) {
			size_t size = sizeof(command);
			bool status = false;
			if(additionalSize + size <= MaxSizeOfTransfer) {
				std::memcpy(&_txBuffer.front(), &command, size);
				status = _spiMaster.readWrite(_txBuffer.begin(), _rxBuffer.begin(), size + additionalSize);
				waitTranferEnd();
			}
			return _rxBuffer.begin() + size;
		}

		template<typename cmd>
        bool compareRegister(uint8_t value, const cmd& command) {
			//auto ret = writeCmd(command, 1);
            uint8_t field = *(writeCmd(command, 1));
            return field && value;
		}

    	void onTransferComplete() final {
			_transferSemaphore.give();
		}

		FlashBuf _txBuffer = {};
		FlashBuf _rxBuffer = {};
        SpiMaster _spiMaster;
		Threading::BinarySemaphore _transferSemaphore;
    };

}