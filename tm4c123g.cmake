#Set cross compilation information
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_BUILD_TYPE Debug)

# GCC toolchain prefix
set(TOOLCHAIN_PREFIX "arm-none-eabi")

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-as)
set(CMAKE_AR ${TOOLCHAIN_PREFIX}-ar)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}-objcopy)
set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}-objdump)


enable_language(ASM)

set(OPT "-Ofast")
set(CPU "-mcpu=cortex-m4")
set(FPU "-mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -mthumb ${CPU}  ${FPU} -MD")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthumb ${CPU} ${FPU} -std=c11 ${OPT} -ffunction-sections -fdata-sections -MD -Wall -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthumb ${CPU} ${FPU}  ${OPT} -ffunction-sections -fdata-sections -MD -Wall -pedantic -fno-exceptions -fno-rtti")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-T${PROJECT_SOURCE_DIR}/tm4c123g.ld -specs=${PROJECT_SOURCE_DIR}/tiva.specs")

# Processor specific definitions
add_definitions(-DPART_TM4C123GH6PM)
add_definitions(-DTARGET_IS_TM4C123_RA1)
add_definitions(-Dgcc)


set(FLASH_EXECUTABLE "openocd")
ADD_CUSTOM_TARGET("flash" DEPENDS ${CMAKE_PROJECT_NAME}.axf
  COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.axf ${CMAKE_PROJECT_NAME}.bin
  COMMAND ${FLASH_EXECUTABLE} -f ${PROJECT_SOURCE_DIR}/ek-tm4c123gxl.cfg -c "program ${CMAKE_PROJECT_NAME}.axf verify reset exit"
)
