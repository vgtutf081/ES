set(LIBRARY_NAME ch32vSdkFreertos)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/RISC-V
        ${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/portable/MemMang
        ${CMAKE_CURRENT_SOURCE_DIR}/portable/Common
)

#source
target_sources(${LIBRARY_NAME} 
    INTERFACE 
        croutine.c
        event_groups.c
        list.c
        queue.c
        stream_buffer.c
        tasks.c
        timers.c
        portable/GCC/RISC-V/portASM.S
        portable/GCC/RISC-V/port.c
        portable/MemMang/heap_4.c
)