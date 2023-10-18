set(PROJECT_NAME Threading)
add_library(${PROJECT_NAME} INTERFACE )

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        "${CMAKE_CURRENT_SOURCE_DIR}/ThreadFreeRtos"
)

target_sources(${PROJECT_NAME} 
    INTERFACE
    ThreadFreeRtos/ThreadFreeRtos.h
    ThreadFreeRtos/CriticalSection.h
    ThreadFreeRtos/Semaphore.h
    ThreadFreeRtos/FreeRtosQueue.h
    ThreadFreeRtos/ThreadFreeRtos.cpp
)

target_compile_definitions(Threading INTERFACE
    THREADING
)
