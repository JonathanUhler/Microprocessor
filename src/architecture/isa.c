#include "architecture/isa.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static const struct isa_register_map isa_register_table[] = {
    /*  ABI          Raw  */
    {"zero", ZERO}, {"r0", R0},
    {"ra", RA},   {"r1", R1},
    {"sp", SP},   {"r2", R2},
    {"a0", A0},   {"r3", R3},
    {"a1", A1},   {"r4", R4},
    {"a2", A2},   {"r5", R5},
    {"a3", A3},   {"r6", R6},
    {"a4", A4},   {"r7", R7},
    {"a5", A5},   {"r8", R8},
    {"a6", A6},   {"r9", R9},
    {"a7", A7},   {"r10", R10},
    {"t0", T0},   {"r11", R11},
    {"t1", T1},   {"r12", R12},
    {"t2", T2},   {"r13", R13},
    {"t3", T3},   {"r14", R14},
    {"t4", T4},   {"r15", R15},
    {"t5", T5},   {"r16", R16},
    {"t6", T6},   {"r17", R17},
    {"t7", T7},   {"r18", R18},
    {"s0", S0},   {"r19", R19},
    {"s1", S1},   {"r20", R20},
    {"s2", S2},   {"r21", R21},
    {"s3", S3},   {"r22", R22},
    {"s4", S4},   {"r23", R23},
    {"s5", S5},   {"r24", R24},
    {"s6", S6},   {"r25", R25},
    {"s7", S7},   {"r26", R26},
    {"s8", S8},   {"r27", R27},
    {"s9", S9},   {"r28", R28},
    {"s10", S10}, {"r29", R29},
    {"s11", S11}, {"r30", R30},
    {"s12", S12}, {"r31", R31}
};


const struct isa_register_map *isa_get_register_map_from_symbol(const char *symbol) {
    size_t n = sizeof(isa_register_table) / sizeof(isa_register_table[0]);
    for (size_t i = 0; i < n; i++) {
        if (strncmp(symbol, isa_register_table[i].symbol, ISA_REGISTER_SYMBOL_MAX_LENGTH) == 0) {
            return &isa_register_table[i];
        }
    }
    return NULL;
}


const struct isa_register_map *isa_get_register_map_from_index(enum isa_register index) {
    size_t n = sizeof(isa_register_table) / sizeof(isa_register_table[0]);
    for (size_t i = 0; i < n; i++) {
        if (index == isa_register_table[i].index) {
            return &isa_register_table[i];
        }
    }
    return NULL;
}
