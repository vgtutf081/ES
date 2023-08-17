include(Sub/Cmake/Toolchain/riscv-none-embed.cmake)

set(MCU_ARCH rv32ecxw)
set(MCU_INTEGER_ABI ilp32e)

set(COMMON_COMPILE_FLAGS -march=${MCU_ARCH}  -mabi=${MCU_INTEGER_ABI} -msmall-data-limit=8)
set(C_CXX_COMPILE_FLAGS  -ffunction-sections -fdata-sections -fsigned-char -g -fmessage-length=0 -Os)

add_library(CH32V0 INTERFACE IMPORTED)

target_compile_options(CH32V0 INTERFACE 
    --sysroot="${TOOLCHAIN_SYSROOT}"
    ${COMMON_COMPILE_FLAGS}
)

target_compile_options(CH32V0 INTERFACE $<$<COMPILE_LANGUAGE:C>:${C_CXX_COMPILE_FLAGS}>)
target_compile_options(CH32V0 INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${C_CXX_COMPILE_FLAGS}>)
target_compile_options(CH32V0 INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics>)


target_compile_options(CH32V0 INTERFACE $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)


target_link_options(CH32V0 INTERFACE 
    --sysroot="${TOOLCHAIN_SYSROOT}"
    ${COMMON_COMPILE_FLAGS}
    ${C_CXX_COMPILE_FLAGS} 
    LINKER:-gc-sections 
    -nostartfiles
)

target_compile_definitions(CH32V0 INTERFACE 
    "__weak=__attribute__((weak))"
    "__packed=__attribute__((__packed__))"
)
