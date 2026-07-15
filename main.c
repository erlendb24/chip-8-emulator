#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "chip_8.h"
#include "raylib.h"

#define INSTRUCTION 2
#define FONT 0x50
#define FONTSIZE 80
#define PROGRAM 0x200
#define MAX_ROW 32
#define MAX_COL 64
#define PIXEL_UPSCALE 10

uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void load_to_ram(cpu *cpu, uint8_t *bytes, int size, int start) {
    for (int i = 0; i < size; i++) {
        cpu->RAM[start + i] = bytes[i];
    }
}

void checkerboard_screen(screen_t *screen) {
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 64; col++) {
            if (row % 2 == 0) {
                if (col % 2 == 0) {
                    (*screen)[col + row * MAX_COL] = 1;
                }
            }
            else if (col % 2 == 1) {
                (*screen)[col + row * MAX_COL] = 1;
            }
        }
    }
}

void draw_screen(screen_t *screen) {
    for (int row = 0; row < MAX_ROW; row++) {
        for (int col = 0; col < MAX_COL; col++) {
            if ((*screen)[col + row * MAX_COL]) {
                DrawRectangle((col * 10), (row * 10), PIXEL_UPSCALE, PIXEL_UPSCALE, BLACK);
            }
        }
    }
}

void play_sound(cpu *cpu, Sound beep) {
    if (cpu->ST > 0) {
        PlaySound(beep);
    } else {
        StopSound(beep);
    }
}

int main(int argc, char **argv) {
    struct stat st;
    //
    FILE* game = fopen(argv[1], "rb");
    stat(argv[1], &st);
    uint16_t instruction = 0;
    cpu cpu = { 0 };
    screen_t screen = { 0 };
    int screen_width = 640;
    int screen_height = 320;
    load_to_ram(&cpu, font, FONTSIZE, FONT);
    fread((cpu.RAM + PROGRAM), 2, st.st_size, game);
    cpu.PC = PROGRAM;

    InitWindow(screen_width, screen_height, "chip-8 emu");
    InitAudioDevice();
    Sound beep = LoadSound("assets/beep-02.wav");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        for (int i = 0; i < 10; i++) {
            instruction = cpu.RAM[cpu.PC] << 8;
            instruction |= cpu.RAM[cpu.PC + 1];
            cpu.PC += 2;
            resolve_instruction(&cpu, instruction, &screen);
        }
        decrement_timers(&cpu);
        play_sound(&cpu, beep);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_screen(&screen);
        EndDrawing();
    }
    CloseWindow();
    fclose(game);
}

