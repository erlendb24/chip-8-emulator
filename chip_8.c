#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "chip_8.h"

#define F 15
#define MAX_COL 64
#define MAX_ROW 32

uint8_t get_byte(uint16_t instruction) {
    uint16_t byte_mask = 0b0000000011111111;
    uint16_t byte = instruction & byte_mask;
    return (uint8_t)byte;
}

uint8_t get_x(uint16_t instruction) {
    uint16_t mask = 0b0000111100000000;
    uint16_t x = (instruction & mask) >> 8;
    return (uint8_t)x;
}

uint8_t get_y(uint16_t instruction) {
    uint16_t mask = 0b0000000011110000;
    uint16_t x = (instruction & mask) >> 4;
    return (uint8_t)x;
}

uint16_t get_addr(uint16_t instruction) {
    uint16_t mask = 0b0000111111111111;
    uint16_t address = instruction;
    return address & mask;
}

uint8_t get_nibble(uint16_t instruction) {
    uint16_t mask = 0b0000000000001111;
    uint16_t nibble = instruction;
    nibble &= mask;
    return (uint8_t)nibble;
}


void jump(cpu *cpu, uint16_t instruction) {
    uint16_t address = get_addr(instruction);
    cpu->PC = address;
}

void jump_plus_v0(cpu *cpu, uint16_t instruction) {
    uint16_t address = get_addr(instruction);
    cpu->PC = address + *cpu->v[0];
}

void randnum(cpu* cpu, uint16_t instruction) {
    uint8_t random = rand() % 256;
    uint8_t reg = get_x(instruction);
    uint8_t byte = get_byte(instruction);
    *cpu->v[reg] = random + byte;
}

void skip_if_equal(cpu *cpu, uint16_t value, char equal) {
    uint8_t reg_val = get_x(value);
    uint8_t compare_val = get_byte(value);
    uint8_t* reg = cpu->v[reg_val];
    if (equal == 1) {
        if (*reg == compare_val) {
            cpu->PC += 2;
        }
    } else {
        if (*reg != compare_val) {
            cpu->PC += 2;
        }
    }
}

void skip_if_reg_equal(cpu *cpu, uint16_t instruction) {
    uint8_t reg_val_1 = get_x(instruction);
    uint8_t reg_val_2 = get_y(instruction);
    if (*cpu->v[reg_val_1] == *cpu->v[reg_val_2]) {
        cpu->PC += 2;
    }
}

void skip_if_reg_not_equal(cpu *cpu, uint16_t instruction) {
    uint8_t reg_val_1 = get_x(instruction);
    uint8_t reg_val_2 = get_y(instruction);
    if (*cpu->v[reg_val_1] != *cpu->v[reg_val_2]) {
        cpu->PC += 2;
    }
}


void set_reg(cpu* cpu, uint16_t instruction) {
    uint8_t reg = get_x(instruction);
    uint8_t byte = get_byte(instruction);
    *cpu->v[reg] = byte;
}

void set_reg_to_reg(cpu* cpu, uint16_t instruction, reg_ops op) {
    uint8_t reg_1 = get_x(instruction);
    uint8_t reg_2 = get_y(instruction);
    uint16_t result = 0;
    switch (op) {
        case NONE:
            *cpu->v[reg_1] = *cpu->v[reg_2];
            break;
        case OR:
            *cpu->v[reg_1] |= *cpu->v[reg_2];
            break;
        case AND:
            *cpu->v[reg_1] &= *cpu->v[reg_2];
            break;
        case XOR:
            *cpu->v[reg_1] ^= *cpu->v[reg_2];
            break;
        case CARRY:
            result = *cpu->v[reg_1] + *cpu->v[reg_2];
            if (result > 255) {
                *cpu->v[F] = 1;
                *cpu->v[reg_1] = get_byte(result); 
            } else {
                *cpu->v[reg_1] = *cpu->v[reg_1] + *cpu->v[reg_2];
                *cpu->v[F] = 0;
            }
            break;
        case SUB:
            if (*cpu->v[reg_1] > *cpu->v[reg_2]) {
                *cpu->v[F] = 1;
            } 
            break;
        default:
            return;
    }
}

void add_to_reg(cpu* cpu, uint16_t instruction) {
    uint8_t reg = get_x(instruction);
    uint8_t byte = get_byte(instruction);
    uint8_t old_val = *cpu->v[reg];
    *cpu->v[reg] = old_val + byte;
}

void set_i_reg(cpu* cpu, uint16_t instruction) {
    uint16_t address = get_addr(instruction);
    cpu->I = address;
}

void draw(cpu *cpu, uint16_t instruction, screen screen) {
    uint8_t amount_of_rows = get_nibble(instruction);
    uint8_t amount_of_cols = 8;
    uint8_t* reg_col = cpu->v[get_x(instruction)];
    uint8_t* reg_row = cpu->v[get_y(instruction)];
    uint8_t* byte = &cpu->RAM[cpu->I];
    uint8_t flag = 0;
    for (int row = 0; row < amount_of_rows; row++) {
        uint16_t mask = 0x80;
        for (int col = 0; col < amount_of_cols; col++) {
            if (*byte & mask) {
                if (screen[(col + *reg_col) % MAX_COL + MAX_COL * ((row + *reg_row) % MAX_ROW)] == 1) {
                    flag = 1;
                }
                screen[col + *reg_col + MAX_COL * (row + *reg_row)] ^= 1;
            }
            mask >>= 1;
        }
        byte++;
    }
    if (flag) {
        *cpu->v[F] = 1;
    } else {
        *cpu->v[F] = 0;
    }
}

void resolve_instruction(cpu *cpu, uint16_t instruction) {
    uint8_t op_code = instruction >> 12;
    reg_ops op = 0;
    switch (op_code) {
        case 1:
            jump(cpu, instruction);
            break;
        case 2:
            //TODO: CALL
            break;
        case 3:
            skip_if_equal(cpu, instruction, 1);
            break;
        case 4:
            skip_if_equal(cpu, instruction, 0);
            break;
        case 5:
            skip_if_reg_equal(cpu, instruction);
            break;
        case 6:
            set_reg(cpu, instruction);
            break;
        case 7:
            add_to_reg(cpu, instruction);
            break;
        case 8:
            // move it forwards to last 4 bits are first, 
            // then back so they're the last, kinda sloppy
            op = (instruction << 12) >> 12;
            set_reg_to_reg(cpu, instruction, op);
        case 9:
            skip_if_reg_not_equal(cpu, instruction);
            break;
        case 0xA:
            set_i_reg(cpu, instruction);
            break;
        case 0xB:
            jump_plus_v0(cpu, instruction);
            break;
        case 0xC:
            randnum(cpu, instruction);
            break;
        case 0xD:
            //TODO: draw
            break;
        case 0xE:
            break;
        case 0xF:
            break;
        default:
            break;
    }
}
