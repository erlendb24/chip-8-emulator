#include <stdio.h>
#include <stdint.h>

#define INSTRUCTION 2


int main(int argc, char** argv) {
    FILE* file = fopen(argv[1], "r");
    uint16_t instruction = 0;
    char *buf;
    while (1) {
        fread(buf, INSTRUCTION, 1, file);
        instruction = buf[0] << 8;
        instruction |= buf[1];
        resolve_instruction(instruction);
    }
}
