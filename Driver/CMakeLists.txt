set(LIBRARY_NAME Driver)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME}
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/Gpio"
        "${CMAKE_CURRENT_SOURCE_DIR}/Uart"
        "${CMAKE_CURRENT_SOURCE_DIR}/GsmModule"        
)

#source
target_sources(${LIBRARY_NAME} 
    INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Uart/UarteNrf52.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Gpio/GpioteNrf52.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Gpio/GpioteNrf52.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Uart/UarteNrf52.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gpio/GpioNrf52.h
)