set(PROJECT_NAME Tools)
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(${PROJECT_NAME} 
    INTERFACE
    CommonTools.h
)

