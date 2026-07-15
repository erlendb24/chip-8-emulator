#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "chip_8.h"
#include "raylib.h"

#define F 15
#define MAX_COL 64
#define MAX_ROW 32

const uint8_t keyboard_lut[16] = {
    KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR,
    KEY_Q, KEY_W, KEY_E, KEY_R,
    KEY_A, KEY_S, KEY_D, KEY_F,
    KEY_Z, KEY_X, KEY_C, KEY_V
};

uint8_t get_byte(uint16_t instruction) {
    uint16_t byte_mask = 0xFF;
    uint16_t byte = instruction & byte_mask;
    return (uint8_t)byte;
}

uint8_t get_x(uint16_t instruction) {
    uint16_t mask = 0xF00;
    uint16_t x = (instruction & mask) >> 8;
    return (uint8_t)x;
}

uint8_t get_y(uint16_t instruction) {
    uint16_t mask = 0xF0;
    uint16_t x = (instruction & mask) >> 4;
    return (uint8_t)x;
}

uint16_t get_addr(uint16_t instruction) {
    uint16_t mask = 0xFFF;
    uint16_t address = instruction;
    return address & mask;
}

uint8_t get_nibble(uint16_t instruction) {
    uint16_t mask = 0xF;
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
    cpu->PC = address + cpu->v[0];
}

void randnum(cpu* cpu, uint16_t instruction) {
    uint8_t random = rand() % 256;
    uint8_t reg = get_x(instruction);
    uint8_t byte = get_byte(instruction);
    cpu->v[reg] = random & byte;
}

void skip_if_equal(cpu *cpu, uint16_t value, char equal) {
    uint8_t reg_val = get_x(value);
    uint8_t compare_val = get_byte(value);
    uint8_t reg = cpu->v[reg_val];
    if (equal == 1) {
        if (reg == compare_val) {
            cpu->PC += 2;
        }
    } else {
        if (reg != compare_val) {
            cpu->PC += 2;
        }
    }
}

void skip_if_reg_equal(cpu *cpu, uint16_t instruction) {
    uint8_t reg_val_1 = get_x(instruction);
    uint8_t reg_val_2 = get_y(instruction);
    if (cpu->v[reg_val_1] == cpu->v[reg_val_2]) {
        cpu->PC += 2;
    }
}

void skip_if_reg_not_equal(cpu *cpu, uint16_t instruction) {
    uint8_t reg_val_1 = get_x(instruction);
    uint8_t reg_val_2 = get_y(instruction);
    if (cpu->v[reg_val_1] != cpu->v[reg_val_2]) {
        cpu->PC += 2;
    }
}


void set_reg(cpu* cpu, uint16_t instruction) {
    uint8_t reg = get_x(instruction);
    uint8_t byte = get_byte(instruction);
    cpu->v[reg] = byte;
}

void set_reg_to_reg(cpu* cpu, uint16_t instruction, reg_ops op) {
    uint8_t reg_x = get_x(instruction);
    uint8_t reg_y = get_y(instruction);
    uint8_t shl_mask = 0x80;
    uint16_t result = 0;
    switch (op) {
        case NONE:
            cpu->v[reg_x] = cpu->v[reg_y];
            break;
        case OR:
            cpu->v[reg_x] |= cpu->v[reg_y];
            break;
        case AND:
            cpu->v[reg_x] &= cpu->v[reg_y];
            break;
        case XOR:
            cpu->v[reg_x] ^= cpu->v[reg_y];
            break;
        case CARRY:
            result = cpu->v[reg_x] + cpu->v[reg_y];
            if (result > 255) {
                cpu->v[F] = 1;
                cpu->v[reg_x] = get_byte(result); 
            } else {
                cpu->v[reg_x] = cpu->v[reg_x] + cpu->v[reg_y];
                cpu->v[F] = 0;
            }
            break;
        case SUB:
            if (cpu->v[reg_x] > cpu->v[reg_y]) {
                cpu->v[F] = 1;
            } else {
                cpu->v[F] = 0;
            }
            cpu->v[reg_x] -= cpu->v[reg_y];
            break;
        case SHR:
            if (cpu->v[reg_x] & 1) {
                cpu->v[F] = 1;
            } else {
                cpu->v[F] = 0;
            }
            cpu->v[reg_x] >>= 1;
            break;
        case SUBN:
            if (cpu->v[reg_x] < cpu->v[reg_y]) {
                cpu->v[F] = 1;
            } else {
                cpu->v[F] = 0;
            }
            cpu->v[reg_x] = cpu->v[reg_y] - cpu->v[reg_x];
            break;
        case SHL:
            if (cpu->v[reg_x] & shl_mask) {
                cpu->v[F] = 1;
            } else {
                cpu->v[F] = 0;
            }
            cpu->v[reg_x] <<= 1;
            break;
        default:
            return;
    }
}

void add_to_reg(cpu* cpu, uint16_t instruction) {
    uint8_t reg = get_x(instruction);
    uint8_t byte = get_byte(instruction);
    uint8_t old_val = cpu->v[reg];
    cpu->v[reg] = old_val + byte;
}

void set_i_reg(cpu* cpu, uint16_t instruction) {
    uint16_t address = get_addr(instruction);
    cpu->I = address;
}

void draw(cpu *cpu, uint16_t instruction, screen_t *screen) {
    uint8_t amount_of_rows = get_nibble(instruction);
    uint8_t amount_of_cols = 8;
    uint8_t reg_col = cpu->v[get_x(instruction)];
    uint8_t reg_row = cpu->v[get_y(instruction)];
    uint8_t* byte = &cpu->RAM[cpu->I];
    uint8_t flag = 0;
    for (int row = 0; row < amount_of_rows; row++) {
        uint16_t mask = 0x80;
        for (int col = 0; col < amount_of_cols; col++) {
            if (*byte & mask) {
                if ((*screen)[(col + reg_col) % MAX_COL + MAX_COL * ((row + reg_row) % MAX_ROW)] == 1) {
                    flag = 1;
                }
                int result = col + reg_col + MAX_COL * (row + reg_row);
                if (result > 0 && result < 2047) {
                    (*screen)[result] ^= 1;
                }
            }
            mask >>= 1;
        }
        byte++;
    }
    if (flag) {
        cpu->v[F] = 1;
    } else {
        cpu->v[F] = 0;
    }
}

void call(cpu *cpu, uint16_t instruction) {
    uint16_t addr = get_addr(instruction);
    cpu->stack[++cpu->SP] = cpu->PC;
    cpu->PC = addr;
}

void return_sub(cpu *cpu) {
    cpu->PC = cpu->stack[cpu->SP];
    cpu->SP--;
}

void clear(screen_t *screen) {
    memset(screen, 0, 2048);
}

int return_lut_index(int target) {
    for (int i = 0; i < 16; i++) {
        if (keyboard_lut[i] == target) {
            return i;
        }
    }
    return -1;
}

void pressed_or_not_pressed(cpu *cpu, uint16_t instruction) {
    uint16_t x = get_x(instruction);
    uint8_t nibble = get_nibble(instruction);
    if (nibble == 0xE) {
        if (IsKeyDown(keyboard_lut[cpu->v[x]])) {
            cpu->PC += 2;
        }
    } else {
        if (!IsKeyDown(keyboard_lut[cpu->v[x]])) {
            cpu->PC += 2;
        }
    }
}

void set_I(cpu *cpu, uint16_t instruction) {
    uint16_t x = get_x(instruction);
    cpu->I += cpu->v[x];
}

void set_I_sprite(cpu *cpu, uint16_t instruction) {
    uint16_t x = get_x(instruction);
    uint16_t start = 0x50;
    uint8_t offset = 5;
    cpu->I = start + offset * cpu->v[x];
}

void set_reg_to_delay(cpu *cpu, uint16_t instruction) {
    uint8_t x = get_x(instruction);
    cpu->v[x] = cpu->DT;
}

void store_key_press(cpu *cpu, uint16_t instruction) {
    uint16_t x = get_x(instruction);
    uint8_t key = 0;
    int8_t chip_8_key = 0;
    while(key == 0) {
        PollInputEvents();
        key = GetKeyPressed();
        chip_8_key = return_lut_index(key);
        if (chip_8_key >= 0) {
            cpu->v[x] = chip_8_key;
        }
    }
}

void set_delay_to_reg(cpu *cpu, uint16_t instruction) {
    uint8_t reg_x = get_x(instruction);
    cpu->DT = cpu->v[reg_x];
}

void set_sound_to_reg(cpu *cpu, uint16_t instruction) {
    uint8_t reg_x = get_x(instruction);
    cpu->ST = cpu->v[reg_x];
}

void decrement_timers(cpu *cpu) {
    if (cpu->ST > 0) {
        cpu->ST--;
    }
    if (cpu->DT > 0) {
        cpu->DT--;
    }
}

void store_bcd(cpu *cpu, uint16_t instruction) {
    uint16_t reg_x = get_x(instruction);
    cpu->RAM[cpu->I] = cpu->v[reg_x] / 100;
    cpu->RAM[cpu->I + 1] = (cpu->v[reg_x] / 10) % 10;
    cpu->RAM[cpu->I + 2] = cpu->v[reg_x] % 10;
}

void read_regs_into_memory(cpu *cpu, uint16_t instruction) {
    uint16_t reg_x = get_x(instruction);
    for (int i = 0; i <= reg_x; i++) {
        cpu->RAM[cpu->I + i] = cpu->v[i];
    }
}

void read_memory_into_regs(cpu *cpu, uint16_t instruction) {
    uint16_t reg_x = get_x(instruction);
    for (int i = 0; i <= reg_x; i++) {
         cpu->v[i] = cpu->RAM[cpu->I + i];
    }
}

void handle_F_instructions(cpu *cpu, uint16_t instruction) {
    uint8_t nibble = get_nibble(instruction);
    uint8_t y = get_y(instruction);

    switch (y) {
        case 0x0:
            if (nibble == 0x7) {
                set_reg_to_delay(cpu, instruction);
            }
            if (nibble == 0xA) {
                store_key_press(cpu, instruction);
            }
            break;
        case 0x1:
            if (nibble == 0x5) {
                set_delay_to_reg(cpu, instruction);
            }
            if (nibble == 0x8) {
                set_sound_to_reg(cpu, instruction);
            }
            if (nibble == 0xE) {
                set_I(cpu, instruction);
            }
            break;
        case 0x2:
            break;
        case 0x3:
            store_bcd(cpu, instruction);
            break;
        case 0x5:
            read_regs_into_memory(cpu, instruction);
            break;
        case 0x6:
            read_memory_into_regs(cpu, instruction);
            break;
        default:
            break;
    }
}

void resolve_instruction(cpu *cpu, uint16_t instruction, screen_t *screen) {
    uint8_t op_code = instruction >> 12;
    reg_ops op = 0;
    uint8_t nibble = get_nibble(instruction);
    switch (op_code) {
        case 0:
            if (nibble == 0) {
                clear(screen);
            } else if (nibble == 0xE) {
                return_sub(cpu);
            }
            break;
        case 1:
            jump(cpu, instruction);
            break;
        case 2:
            call(cpu, instruction);
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
            op = get_nibble(instruction);
            set_reg_to_reg(cpu, instruction, op);
            break;
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
            draw(cpu, instruction, screen);
            break;
        case 0xE:
            pressed_or_not_pressed(cpu, instruction);
            break;
        case 0xF:
            handle_F_instructions(cpu, instruction);
            break;
        default:
            break;
    }
}
