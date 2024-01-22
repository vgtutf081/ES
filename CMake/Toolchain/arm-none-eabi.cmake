if(EXISTS "C:/Program Files (x86)/GNU Arm Embedded Toolchain/10 2021.10")
    set(TOOLCHAIN_DIR "C:/Program Files (x86)/GNU Arm Embedded Toolchain/10 2021.10")
elseif(EXISTS "C:/GNU Arm Embedded Toolchain/10 2021.10")
    set(TOOLCHAIN_DIR "C:/GNU Arm Embedded Toolchain/10 2021.10")
else()
    message(FATAL_ERROR "Toolchain not found!")
endif()

message("TOOLCHAIN_DIR: ${TOOLCHAIN_DIR}")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)

set(MCU_ARCH cortex-m4)

set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_TRIPLET "arm-none-eabi")
set(CMAKE_LIBRARY_ARCHITECTURE arm-none-eabi)

include(${CMAKE_CURRENT_LIST_DIR}/gcc.cmake)
