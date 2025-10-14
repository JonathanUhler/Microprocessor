#ifndef _ARCHITECTURE_ISA_H_
#define _ARCHITECTURE_ISA_H_


#include <stdint.h>
#include <stdlib.h>


#define ISA_REGISTER_SYMBOL_MAX_LENGTH 7
#define ISA_OPCODE_SYMBOL_MAX_LENGTH 7

#define ISA_INSTRUCTION_FORMAT_SIZE     2
#define ISA_INSTRUCTION_FUNCT_SIZE      4
#define ISA_INSTRUCTION_REGISTER_SIZE   5
#define ISA_INSTRUCTION_IMMEDIATE_SIZE 16

#define ISA_INSTRUCTION_FORMAT_MASK ((1U << ISA_INSTRUCTION_FORMAT_SIZE) - 1U)


enum isa_register {
    ZERO = 0,  R0  = 0,
    RA   = 1,  R1  = 1,
    SP   = 2,  R2  = 2,
    A0   = 3,  R3  = 3,
    A1   = 4,  R4  = 4,
    A2   = 5,  R5  = 5,
    A3   = 6,  R6  = 6,
    A4   = 7,  R7  = 7,
    A5   = 8,  R8  = 8,
    A6   = 9,  R9  = 9,
    A7   = 10, R10 = 10,
    T0   = 11, R11 = 11,
    T1   = 12, R12 = 12,
    T2   = 13, R13 = 13,
    T3   = 14, R14 = 14,
    T4   = 15, R15 = 15,
    T5   = 16, R16 = 16,
    T6   = 17, R17 = 17,
    T7   = 18, R18 = 18,
    S0   = 19, R19 = 19,
    S1   = 20, R20 = 20,
    S2   = 21, R21 = 21,
    S3   = 22, R22 = 22,
    S4   = 23, R23 = 23,
    S5   = 24, R24 = 24,
    S6   = 25, R25 = 25,
    S7   = 26, R26 = 26,
    S8   = 27, R27 = 27,
    S9   = 28, R28 = 28,
    S10  = 29, R29 = 29,
    S11  = 30, R30 = 30,
    S12  = 31, R31 = 31
};


struct isa_register_map {
    const char *symbol;
    enum isa_register index;
};


struct isa_i_format {
    uint16_t format       : ISA_INSTRUCTION_FORMAT_SIZE;
    uint16_t funct        : ISA_INSTRUCTION_FUNCT_SIZE;
    uint16_t __RESERVED__ : 10;
    uint16_t immediate    : ISA_INSTRUCTION_IMMEDIATE_SIZE;
} __attribute__((packed));


struct isa_dsi_format {
    uint16_t format    : ISA_INSTRUCTION_FORMAT_SIZE;
    uint16_t funct     : ISA_INSTRUCTION_FUNCT_SIZE;
    uint16_t dest      : ISA_INSTRUCTION_REGISTER_SIZE;
    uint16_t source1   : ISA_INSTRUCTION_REGISTER_SIZE;
    uint16_t immediate : ISA_INSTRUCTION_IMMEDIATE_SIZE;
} __attribute__((packed));


struct isa_dss_format {
    uint16_t format       : ISA_INSTRUCTION_FORMAT_SIZE;
    uint16_t funct        : ISA_INSTRUCTION_FUNCT_SIZE;
    uint16_t dest         : ISA_INSTRUCTION_REGISTER_SIZE;
    uint16_t source1      : ISA_INSTRUCTION_REGISTER_SIZE;
    uint16_t source2      : ISA_INSTRUCTION_REGISTER_SIZE;
    uint16_t __RESERVED__ : 11;
} __attribute__((packed));


union isa_instruction {
    struct isa_i_format i_type;
    struct isa_dsi_format dsi_type;
    struct isa_dss_format dss_type;
    uint32_t binary;
};


enum isa_opcode {
    // First row (I-Type)
    HALT = 0b000000,

    // Third row (DSI-Type)
    ADDI = 0b000010,
    SUBI = 0b000110,
    ANDI = 0b001010,
    ORI  = 0b001110,
    XORI = 0b010010,
    SLLI = 0b010110,
    SRLI = 0b011010,
    SRAI = 0b011110,
    LD   = 0b110010,
    ST   = 0b110110,
    JLZ  = 0b111010,
    JLO  = 0b111110,

    // Fourth row (DSS-Type)
    ADD  = 0b000011,
    SUB  = 0b000111,
    AND  = 0b001011,
    OR   = 0b001111,
    XOR  = 0b010011,
    SLL  = 0b010111,
    SRL  = 0b011011,
    SRA  = 0b011111,
    EQ   = 0b100011,
    GT   = 0b100111,
    LT   = 0b101011,
    NE   = 0b101111,
    JLRZ = 0b111011,
    JLRO = 0b111111
};


enum isa_opcode_format {
    ISA_OPCODE_FORMAT_I      = 0b00,
    ISA_OPCODE_FORMAT_DSI    = 0b10,
    ISA_OPCODE_FORMAT_DSS    = 0b11,
    ISA_OPCODE_FORMAT_PSEUDO = 0xFF
};


struct isa_opcode_map {
    const char *symbol;
    enum isa_opcode opcode;
    enum isa_opcode_format format;
    uint8_t funct;
};


const struct isa_register_map *isa_get_register_map_from_symbol(const char *symbol);


const struct isa_register_map *isa_get_register_map_from_index(enum isa_register index);


const struct isa_opcode_map *isa_get_opcode_map_from_symbol(const char *symbol);


const struct isa_opcode_map *isa_get_opcode_map_from_opcode(enum isa_opcode opcode);


#endif  // _ARCHITECTURE_ISA_H_
