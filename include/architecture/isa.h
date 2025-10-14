#ifndef _ARCHITECTURE_ISA_H_
#define _ARCHITECTURE_ISA_H_


/** The maximum length (EXCLUDING the null terminator) for regsiter ABI names. */
#define ISA_REGISTER_SYMBOL_MAX_LENGTH 7


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


const struct isa_register_map *isa_get_register_map_from_symbol(const char *symbol);


const struct isa_register_map *isa_get_register_map_from_index(enum isa_register index);


#endif  // _ARCHITECTURE_ISA_H_
