add_library(CH32V INTERFACE IMPORTED)

target_compile_options(CH32V INTERFACE 
    --sysroot="${TOOLCHAIN_SYSROOT}"
    ${COMMON_COMPILE_FLAGS}
)

target_compile_options(CH32V INTERFACE $<$<COMPILE_LANGUAGE:C>:${C_CXX_COMPILE_FLAGS}>)
target_compile_options(CH32V INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${C_CXX_COMPILE_FLAGS}>)
target_compile_options(CH32V INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics>)

target_compile_options(CH32V INTERFACE $<$<COMPILE_LANGUAGE:C>:-Og -g>)
target_compile_options(CH32V INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Og -g>)
target_compile_options(CH32V INTERFACE $<$<COMPILE_LANGUAGE:ASM>:-g>)

target_compile_options(CH32V INTERFACE $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>)


target_link_options(CH32V INTERFACE 
    --sysroot="${TOOLCHAIN_SYSROOT}"
    ${COMMON_COMPILE_FLAGS}
    ${C_CXX_COMPILE_FLAGS} 
    LINKER:-gc-sections 
    -nostartfiles
)

target_compile_definitions(CH32V INTERFACE 
    "__weak=__attribute__((weak))"
    "__packed=__attribute__((__packed__))"
    CH32V
)