set(TOOLCHAIN_PREFIX "${CMAKE_LIBRARY_ARCHITECTURE}-")
set(TOOLCHAIN_BIN_PATH "${TOOLCHAIN_DIR}/bin")
set(TOOLCHAIN_INC_PATH "${TOOLCHAIN_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/include")
set(TOOLCHAIN_LIB_PATH "${TOOLCHAIN_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}/lib")
set(TOOLCHAIN_SYSROOT  "${TOOLCHAIN_DIR}/${CMAKE_LIBRARY_ARCHITECTURE}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
#message(WARNING "TOOLCHAIN_BIN_PATH: ${TOOLCHAIN_BIN_PATH}")

set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}gcc.exe)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}g++.exe)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}gcc.exe)
set(CMAKE_CPPFILT ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}c++filt.exe)
set(CMAKE_DEBUGGER ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}gdb.exe)
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}objcopy.exe)
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}objdump.exe)
set(CMAKE_SIZE ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}size.exe)
set(CMAKE_AS ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}as.exe)
set(CMAKE_AR ${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}ar.exe)

set(CMAKE_EXECUTABLE_SUFFIX_C   .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf)

add_library(NOSYS INTERFACE IMPORTED)
target_link_options(NOSYS INTERFACE 
    -specs=nosys.specs
)

add_library(NANO INTERFACE IMPORTED)
target_link_options(NANO INTERFACE 
    -specs=nano.specs
)


function(gcc_print_target_size TargetName)
    add_custom_command(
        TARGET ${TargetName}
        POST_BUILD
	    COMMAND ${CMAKE_COMMAND} -E echo "Invoking: GCC Print Size"
        COMMAND ${CMAKE_SIZE} ${TargetName}${CMAKE_EXECUTABLE_SUFFIX_C}
    )
endfunction()

function(gcc_generate_bin_file TargetName)
    add_custom_command(
        TARGET ${TargetName}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Generating flash image ${TargetName}.bin"
        COMMAND ${CMAKE_OBJCOPY} -O binary ${TargetName}${CMAKE_EXECUTABLE_SUFFIX_C} ${TargetName}.bin
        BYPRODUCTS ${TargetName}.bin
    )
endfunction()


function(gcc_generate_hex_file TargetName)
    add_custom_command(
        TARGET ${TargetName}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Generating flash image ${TargetName}.hex"
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${TargetName}${CMAKE_EXECUTABLE_SUFFIX_C} ${TargetName}.hex
        BYPRODUCTS ${TargetName}.hex
    )
endfunction()


function(gcc_generate_srec_file TargetName)
    add_custom_command(
        TARGET ${TargetName}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Generating flash image ${TargetName}.srec"
        COMMAND ${CMAKE_OBJCOPY} -O srec ${TargetName}${CMAKE_EXECUTABLE_SUFFIX_C} ${TargetName}.srec
        BYPRODUCTS ${TargetName}.srec
    )
endfunction()

function(gcc_add_linker_script TARGET VISIBILITY SCRIPT_PATH)
    get_filename_component(SCRIPT_PATH "${SCRIPT_PATH}" ABSOLUTE)
    target_link_options(${TARGET} ${VISIBILITY} -T "${SCRIPT_PATH}")
endfunction()

