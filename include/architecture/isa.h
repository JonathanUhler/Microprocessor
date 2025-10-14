/**
 * Definition of the instruction set architecture through enums and structures.
 *
 * @author Jonathan Uhler
 */


#ifndef _ARCHITECTURE_ISA_H_
#define _ARCHITECTURE_ISA_H_


#include <stdint.h>
#include <stdlib.h>


/** The maximum length of a symbolic register name (EXCLUDING the null terminator). */
#define ISA_REGISTER_SYMBOL_MAX_LENGTH 7
/** THe maximum length of a symbolic opcode name (EXCLUDING the null terminator). */
#define ISA_OPCODE_SYMBOL_MAX_LENGTH 7

/** The number of bits in the Format field of an instruction. */
#define ISA_INSTRUCTION_FORMAT_SIZE     2
/** The number of bits in the Funct field of an instruction. */
#define ISA_INSTRUCTION_FUNCT_SIZE      4
/** The number of bits in the Dest/Source1/Source2 fields of an instruction. */
#define ISA_INSTRUCTION_REGISTER_SIZE   5
/** The number of bits in the Immediate field of an instruction. */
#define ISA_INSTRUCTION_IMMEDIATE_SIZE 16

/** Bitmask for the Format field of an instruction. */
#define ISA_INSTRUCTION_FORMAT_MASK ((1U << ISA_INSTRUCTION_FORMAT_SIZE) - 1U)


/**
 * Enumeration of the index (binary) values of all registers (ABI names and raw names).
 */
enum isa_register {
    /* ABI       Raw */
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


/**
 * A mapping/pair of a register's symbolic name (ABI or raw) as a string and its index (binary).
 */
struct isa_register_map {
    /** The symbolic name (ABI or raw) of the register. */
    const char *symbol;
    /** The index (binary) value of the register. */
    enum isa_register index;
};


/**
 * A packed 32-bit word representing an I-type instruction.
 */
struct isa_i_format {
    /** The Format field (1:0). */
    uint16_t format       : ISA_INSTRUCTION_FORMAT_SIZE;
    /** The Funct field (5:2). */
    uint16_t funct        : ISA_INSTRUCTION_FUNCT_SIZE;
    /** Unused bits (15:6). */
    uint16_t __RESERVED__ : 10;
    /** The Immediate field (31:16). */
    uint16_t immediate    : ISA_INSTRUCTION_IMMEDIATE_SIZE;
} __attribute__((packed));


/**
 * A packed 32-bit word representing a DSI-type instruction.
 */
struct isa_dsi_format {
    /** The Format field (1:0). */
    uint16_t format    : ISA_INSTRUCTION_FORMAT_SIZE;
    /** The Funct field (5:2). */
    uint16_t funct     : ISA_INSTRUCTION_FUNCT_SIZE;
    /** The Dest field (10:6). */
    uint16_t dest      : ISA_INSTRUCTION_REGISTER_SIZE;
    /** The Source1 field (15:11). */
    uint16_t source1   : ISA_INSTRUCTION_REGISTER_SIZE;
    /** The Immediate field (31:16). */
    uint16_t immediate : ISA_INSTRUCTION_IMMEDIATE_SIZE;
} __attribute__((packed));


/**
 * A packed 32-bit word representing a DSS-type instruction.
 */
struct isa_dss_format {
    /** The Format field (1:0). */
    uint16_t format       : ISA_INSTRUCTION_FORMAT_SIZE;
    /** The Funct field (5:2). */
    uint16_t funct        : ISA_INSTRUCTION_FUNCT_SIZE;
    /** The Dest field (10:6). */
    uint16_t dest         : ISA_INSTRUCTION_REGISTER_SIZE;
    /** The Source1 field (15:11). */
    uint16_t source1      : ISA_INSTRUCTION_REGISTER_SIZE;
    /** The Source2 field (20:16). */
    uint16_t source2      : ISA_INSTRUCTION_REGISTER_SIZE;
    /**Unused bits (31:21). */
    uint16_t __RESERVED__ : 11;
} __attribute__((packed));


/**
 * A view of an instruction's binary value and all three possible format encodings of it.
 */
union isa_instruction {
    /** The instruction viewed as an I-type. */
    struct isa_i_format i_type;
    /** The instruction viewed as a DSI-type. */
    struct isa_dsi_format dsi_type;
    /** The instruction viewed as a DSS-type. */
    struct isa_dss_format dss_type;
    /** The binary (little-endian) of the instruction. */
    uint32_t binary;
};


/**
 * Enumeration of all the opcodes for core instructions.
 */
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


/**
 * Enumeration of all possible values for the Format (1:0) field of an instruction.
 */
enum isa_opcode_format {
    /** The format of an I-type instruction. */
    ISA_OPCODE_FORMAT_I      = 0b00,
    /** The format of a DS-type instruction. */
    ISA_OPCODE_FORMAT_DSI    = 0b10,
    /** The format of a DSS-type instruction. */
    ISA_OPCODE_FORMAT_DSS    = 0b11,
    /** A convenience marker for pseudo instructions (not used in the actual binary). */
    ISA_OPCODE_FORMAT_PSEUDO = 0xFF
};


/**
 * A mapping/pair of an opcode's symbolic name as a string, its value in binary, and its Format
 * and Funct fields.
 */
struct isa_opcode_map {
    /** The symbolic name of the opcode. */
    const char *symbol;
    /** The binary value of the opcode. */
    enum isa_opcode opcode;
    /** The format of the instruction (opcode[1:0]). */
    enum isa_opcode_format format;
    /** The function of the instruction (opcode[5:2]). */
    uint8_t funct;
};


/**
 * Gets a register mapping from a register's symbolic name.
 *
 * @param symbol  The symbolic name of the register (ABI or raw).
 *
 * @return The register mapping for the provided symbolic name.
 */
const struct isa_register_map *isa_get_register_map_from_symbol(const char *symbol);


/**
 * Gets a register mapping from a register's binary index.
 *
 * @param index  The binary value of the register.
 *
 * @return The register mapping for the provided symbolic name. The regsiter mapping entry for
 *         the ABI name will be preferred when available.
 */
const struct isa_register_map *isa_get_register_map_from_index(enum isa_register index);


/**
 * Gets an opcode mapping from an opcode's symbolic name.
 *
 * @param symbol  The symbolic name of the opcode (which can be a pseudo-opcode).
 *
 * @return The opcode mapping for the provided symbolic name. If the provided symbol is a pseudo-
 *         instruction, the Funct and Format fields in the returned map are ISA_OPCODE_FORMAT_PSEUDO
 */
const struct isa_opcode_map *isa_get_opcode_map_from_symbol(const char *symbol);


/**
 * Gets an opcode mapping from an opcode's binary value.
 *
 * @param opcode  The binary value of the opcode (which must be a core instruction).
 *
 * @return The opcode mapping for the provided opcode value. Only core instruction mappings will
 *         be returned.
 */
const struct isa_opcode_map *isa_get_opcode_map_from_opcode(enum isa_opcode opcode);


#endif  // _ARCHITECTURE_ISA_H_
