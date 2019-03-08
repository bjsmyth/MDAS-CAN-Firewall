cmake_minimum_required(VERSION 3.5.0)
project(mdas_firewall)

#Toolchain file
include(tm4c123g.cmake)

#Tivaware files
#set(TIVAWARE_PATH "$ENV{HOME}/tiva-projects/tivaware")
#include_directories(${TIVAWARE_PATH})
include_directories(${${CMAKE_PROJECT_NAME}_SOURCE_DIR}/include)

#Source files
file(GLOB SOURCES "include/*.c" "include/*/*.c" "src/*/*.c" "src/*.c")
add_executable(${CMAKE_PROJECT_NAME}.axf ${SOURCES})

target_link_libraries(${CMAKE_PROJECT_NAME}.axf
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME}.axf POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --yellow "==============================Memory Sizes=============================="
  #COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "========================Text=ROM Data,BSS=SRAM=========================="
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --yellow " "
  COMMAND ${TOOLCHAIN_PREFIX}-size --format=berkeley "${CMAKE_PROJECT_NAME}.axf"
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --yellow " "
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --yellow "========================================================================"
)
