#include <stdio.h>
#include <stdint.h>

typedef struct cpu {
    unsigned char* v1;
    unsigned char* v2;
    unsigned char* v3;
    unsigned char* v4;
    unsigned char* v5;
    unsigned char* v6;
    unsigned char* v7;
    unsigned char* v8;
    unsigned char* v9;
    unsigned char* vA;
    unsigned char* vB;
    unsigned char* vC;
    unsigned char* vD;
    unsigned char* vE;
    unsigned char* vF;
    uint16_t PC;
} cpu;


unsigned char* resolve_reg(cpu *cpu, unsigned char reg) {
    switch (reg) {
        case 1:
            return cpu->v1;
        case 2:
            return cpu->v2;
        case 3:
            return cpu->v3;
        case 4:
            return cpu->v4;
        case 5:
            return cpu->v5;
        case 6:
            return cpu->v6;
        case 7:
            return cpu->v7;
        case 8:
            return cpu->v8;
        case 9:
            return cpu->v9;
        case 10:
            return cpu->vA;
        case 11:
            return cpu->vB;
        case 12:
            return cpu->vC;
        case 13:
            return cpu->vD;
        case 14:
            return cpu->vE;
        case 15:
            return cpu->vF;
        default:
            break;
    }
    return NULL;
}

unsigned char get_byte(uint16_t instruction) {
    uint16_t byte_mask = 0b0000000011111111;
    uint16_t byte = instruction & byte_mask;
    return (unsigned char)byte;
}

unsigned char get_x(uint16_t instruction) {
    uint16_t mask = 0b0000111100000000;
    uint16_t x = instruction & mask;
    return (unsigned char)x;
}

unsigned char get_y(uint16_t instruction) {
    uint16_t mask = 0b0000000011110000;
    uint16_t x = instruction & mask;
    return (unsigned char)x;
}

uint16_t get_addr(uint16_t instruction) {
    uint16_t mask = 0b0000111111111111;
    uint16_t address = 0;
    return address & mask;
}

unsigned char get_nibble(uint16_t instruction) {
    uint16_t mask = 0b0000000000001111;
    uint16_t nibble = 0;
    nibble &= mask;
    return (unsigned char)nibble;
}


void jump(cpu *cpu, uint16_t instruction) {
    uint16_t address = get_addr(instruction);
    cpu->PC = address;
}

void skip_if_equal(cpu *cpu, uint16_t value) {
    uint16_t mask = 0b0000111100000000;
    uint16_t reg_val = (value & mask) >> 8;
    unsigned char compare_val = get_byte(value);
    unsigned char* reg = resolve_reg(cpu, reg_val);
    if (*reg == compare_val) {
        cpu->PC += 2;
    }
}
