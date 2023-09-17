
set(TOOLCHAIN_DIR "C:/MounRiver/MounRiver_Studio/toolchain/RISC-V Embedded GCC")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_SYSTEM_PROCESSOR rv32imac)

set(TOOLCHAIN_TRIPLET "riscv-none-embed")
set(CMAKE_LIBRARY_ARCHITECTURE riscv-none-embed)

include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)


