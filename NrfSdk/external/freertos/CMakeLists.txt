set(LIBRARY_NAME NrfSdkFreertos)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/source/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/portable/CMSIS/nrf52"
        "${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/nrf52" 
        "${CMAKE_CURRENT_SOURCE_DIR}/config" 
)

#source
target_sources(${LIBRARY_NAME} 
INTERFACE 
    source/croutine.c
    source/event_groups.c
    source/list.c
    source/queue.c
    source/readme.txt
    source/stream_buffer.c
    source/tasks.c
    source/timers.c
)

#source/include
target_sources(${LIBRARY_NAME}
INTERFACE 
    source/include/croutine.h
    source/include/deprecated_definitions.h
    source/include/event_groups.h
    source/include/FreeRTOS.h
    source/include/list.h
    source/include/message_buffer.h
    source/include/mpu_prototypes.h
    source/include/mpu_wrappers.h
    source/include/portable.h
    source/include/projdefs.h
    source/include/queue.h
    source/include/semphr.h
    source/include/StackMacros.h
    source/include/stack_macros.h
    source/include/stdint.readme
    source/include/stream_buffer.h
    source/include/task.h
    source/include/timers.h
    source/include/semphr.h
    source/include/queue.h
)


#source/portable/Common
target_sources(${LIBRARY_NAME} 
INTERFACE 
    #source/portable/Common/mpu_wrappers.c
)

#source/portable/MemMang
target_sources(${LIBRARY_NAME} 
INTERFACE 
    source/portable/MemMang/heap_4.c
)

#portable/CMSIS/nrf52
target_sources(${LIBRARY_NAME} 
INTERFACE 
    portable/CMSIS/nrf52/portmacro_cmsis.h
    portable/CMSIS/nrf52/port_cmsis.c
    portable/CMSIS/nrf52/port_cmsis_systick.c
)

#portable/GCC/nrf52
target_sources(${LIBRARY_NAME}
INTERFACE 
    portable/GCC/nrf52/port.c
    portable/GCC/nrf52/portmacro.h
)

#config
target_sources(${LIBRARY_NAME}
INTERFACE 
    config/FreeRTOSConfig.h
)