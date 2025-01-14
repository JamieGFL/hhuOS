cmake_minimum_required(VERSION 3.14)
project(nes)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${HHUOS_SRC_DIR})

# Set source files
set(SOURCE_FILES
        ${HHUOS_SRC_DIR}/application/nes/ppu2C02.c ${HHUOS_SRC_DIR}/application/nes/nesBus.c
        ${HHUOS_SRC_DIR}/application/nes/mapper.c ${HHUOS_SRC_DIR}/application/nes/cartridge.c
        ${HHUOS_SRC_DIR}/application/nes/NESManager.cpp
        ${HHUOS_SRC_DIR}/application/nes/mapper000.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} lib.user.runtime lib.user.base lib.user.cpu6502)

