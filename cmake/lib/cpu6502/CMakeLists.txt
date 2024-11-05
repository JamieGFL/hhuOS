cmake_minimum_required(VERSION 3.14)

project(lib.user.cpu6502)
message(STATUS "Project " ${PROJECT_NAME})

include_directories(${HHUOS_SRC_DIR})

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PUBLIC
        ${HHUOS_SRC_DIR}/lib/cpu6502/cpu6502.cpp)