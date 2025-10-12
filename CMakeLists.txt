cmake_minimum_required(VERSION 3.20)

project(Microprocessor C)

add_compile_options(-std=c11 -Wall -Wextra -Wpedantic -Werror)

set(SRC_DIR src)
set(INCLUDE_DIR include)

include_directories(${INCLUDE_DIR})

add_executable(assembler
  ${SRC_DIR}/assembler/assembler.c
  ${SRC_DIR}/assembler/lexer.c
)
