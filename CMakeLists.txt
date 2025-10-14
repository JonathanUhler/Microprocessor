cmake_minimum_required(VERSION 3.20)

project(Microprocessor C)

add_compile_options(-std=c2x -Wall -Wextra -Wpedantic -Werror)

set(SRC_DIR src)
set(INCLUDE_DIR include)

include_directories(${INCLUDE_DIR})


add_library(architecture STATIC
  ${SRC_DIR}/architecture/isa.c
  ${SRC_DIR}/architecture/logger.c
)

add_executable(assembler
  ${SRC_DIR}/assembler/assembler.c
  ${SRC_DIR}/assembler/lexer.c
  ${SRC_DIR}/assembler/parser.c
  ${SRC_DIR}/assembler/encoder.c
)
target_link_libraries(assembler PRIVATE architecture)

add_executable(simulator
  ${SRC_DIR}/simulator/simulator.c
  ${SRC_DIR}/simulator/memory.c
  ${SRC_DIR}/simulator/registers.c
  ${SRC_DIR}/simulator/processor.c
)
target_link_libraries(simulator PRIVATE architecture)
