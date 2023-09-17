include(ES/CMake/Toolchain/riscv-none-embed.cmake)

set(MCU_ARCH rv32ecxw)
set(MCU_INTEGER_ABI ilp32e)

set(COMMON_COMPILE_FLAGS -march=${MCU_ARCH}  -mabi=${MCU_INTEGER_ABI} -msmall-data-limit=8)
set(C_CXX_COMPILE_FLAGS  -ffunction-sections -fdata-sections -fsigned-char -fmessage-length=0)

include(ES/CMake/CH32/CH32Vcommon.cmake)