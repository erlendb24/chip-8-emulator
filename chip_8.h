#include <stdio.h>
#include <stdint.h>

extern const uint8_t keyboard_lut[16];

typedef struct cpu {
    uint16_t I;
    uint16_t PC;
    uint8_t SP;
    uint8_t DT;
    uint8_t ST;
    uint8_t v[16];
    uint16_t stack[16];
    uint8_t RAM[4096];
} cpu;

typedef uint8_t screen_t[2048];

typedef enum {
    NONE,
    OR,
    AND,
    XOR,
    CARRY,
    SUB,
    SHR,
    SUBN,
    SHL = 14
} reg_ops;

void resolve_instruction(cpu *cpu, uint16_t instruction, screen_t *screen);
void decrement_timers(cpu *cpu);
