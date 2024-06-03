#pragma once

#include "SpiMasterNrf52.h"
#include "Semaphore.h"
#include <array>
#include "ThreadFreeRtos.h"
#include <algorithm>

namespace ES::Driver {
	
    class FM25V10 : public ISpiEventHandler {
    public:
    	static constexpr size_t MaxSizeOfTransfer = 128;
        static constexpr size_t MemorySizeMBit = 1;

		static constexpr uint8_t StatusWel = 0x02;
		static constexpr uint8_t StatusBp0 = 0x04;
		static constexpr uint8_t StatusBp1 = 0x08;
		static constexpr uint8_t WriteProtectEnableBit = 0x80;


		static constexpr uint32_t StartAddress = 0x00000000;
		static constexpr uint32_t MaxAddress = 0x0001FFFF;

		using FlashBuf = std::array<uint8_t, MaxSizeOfTransfer * 2>;

        enum class Commands : uint8_t {
            WriteEnable = 0x06,
            WriteDisable = 0x04,
            ReadStatusRegister1 = 0x05,
            WriteStatusRegister1 = 0x01,
			ReadData = 0x03,
			PageProgram = 0x02,
			EnterSleepMode = 0xC3
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
		
        FM25V10(ISpiMaster &spiMaster) : _spiMaster(spiMaster) {
			if(_spiMaster.setEventHandler(this)) {
				blockingMode = false;
			}
			else {
				blockingMode = true;
			}
        }
        
		void waitTranferEnd() {
			if(!blockingMode) {
				_transferSemaphore.take();
			}
		}

		void enterSleepMode() {
			writeCmd(Cmd{Commands::EnterSleepMode});
		}

		bool chipErase() {
			bool status = true;
			std::array<uint8_t, MaxSizeOfTransfer> eraseBuf;
			std::fill(eraseBuf.begin(), eraseBuf.end(), 0xFF);
			for(uint32_t i = StartAddress; i < MaxAddress - MaxSizeOfTransfer; i += MaxSizeOfTransfer) {
				status &= writePage(i, eraseBuf.begin(), MaxSizeOfTransfer);
			}
			return status;
		}

		bool sectorErase(uint32_t address, size_t size) {
			bool status = true;
			std::array<uint8_t, MaxSizeOfTransfer> eraseBuf;
			std::fill(eraseBuf.begin(), eraseBuf.end(), 0xFF);
			for(uint32_t i = address; i <= address + (MaxSizeOfTransfer * size); i += MaxSizeOfTransfer) {
				status &= writePage(i, eraseBuf.begin(), MaxSizeOfTransfer);
			}
			return status;
		}

		bool writePage(uint32_t address, const uint8_t* data, size_t size) {
			bool status = true;
			if(address + size > MaxAddress) {
				return false;
			}
			status &= tryWriteEnable();
			std::memcpy(&_txBuffer[sizeof(AddressCmd{Commands::PageProgram, address})], data, size);
			writeCmd(AddressCmd{Commands::PageProgram, address}, size);
			auto returnData = readPage(address, size);
			if(std::memcmp(data, returnData, size) != 0) {
				return false;
			}
			return true;
		}

		FlashBuf::const_iterator readPage(uint32_t address, size_t size) {
			if(address + size <= MaxAddress) {
				writeCmd(Cmd{Commands::WriteDisable});
				_txBuffer.fill(0xFF);
				return writeCmd(AddressCmd{Commands::ReadData, address}, size);
			}
			else {
				return nullptr;
			}
		}

    private:

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
			if(additionalSize <= MaxSizeOfTransfer) {
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
            bool status = field && value;
			return status;
		}

    	void onTransferComplete() final {
			if(!blockingMode) {
				_transferSemaphore.give();
			}
		}

		bool blockingMode = false;
		FlashBuf _txBuffer = {};
		FlashBuf _rxBuffer = {};
        //SpiMaster<ISpiMaster::SpiMode::Blocking> _spiMaster;
		ISpiMaster &_spiMaster;
		Threading::BinarySemaphore _transferSemaphore;
    };

}