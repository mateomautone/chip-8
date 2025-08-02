#include <assert.h>
#include <chip8.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Load font data (050-09F apparent convention)
static inline void load_font(Chip8 *chip8) {
  uint8_t fontset[16 * 5] = {
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
  // for (uint16_t i = 0; i < sizeof(fontset); i++) {
  //   chip8->memory[0x50 + i] = fontset[i];
  // }
  memcpy(&chip8->memory[0x50], fontset, sizeof(fontset));
}

// Initialize the CHIP8 values
void chip8_initialize(Chip8 *chip8) {
  *chip8 = (Chip8){0};
  chip8->PC = 0x200;
  load_font(chip8);
}

// Display Initialization
void chip8_display_initialize(Chip8Display *display){
  *display = (Chip8Display){0};
  // memset(display, 0xFA, sizeof(*display));
}

// Print Registers
void chip8_print_registers(Chip8 *chip8) {
  printf("Program Counter\t0x%04x\n", chip8->PC);
  printf("Stack Pointer\t0x%02x\n", chip8->SP);
  printf("Stack\t\t");
  for (int i = 0; i < 16; i++) {
    printf("0x%04x ", chip8->stack[i]);
  }
  printf("\nI Register\t0x%04x\n", chip8->I);
  for (int i = 0; i < 16; i++) {
    printf("V%X Register\t0x%02x\n", i, chip8->V[i]);
  }
  printf("Delay Timer\t0x%02x\n", chip8->DT);
  printf("Sound Timer\t0x%02x\n", chip8->ST);
}

// Hexdump memory region
void chip8_mem_hexdump(Chip8 *chip8, uint16_t start_addr, uint16_t end_addr) {
  assert(start_addr <= end_addr);
  assert(end_addr < CHIP8_MEM_SIZE);
  const int bytes_per_line = 32;
  for (int newline_counter = 0;start_addr<=end_addr;start_addr++) {
    if (newline_counter == bytes_per_line){
      newline_counter = 0;
      printf("\n");
    }
    if (newline_counter == 0)
      printf("0x%04x-0x%04x\t", start_addr, start_addr + bytes_per_line - 1);
    printf("0x%02x ", chip8->memory[start_addr]);
    newline_counter++;
  }
  printf("\n");
}

// Print Display
void chip8_print_display(Chip8Display *display){
  for (int h = 0; h < CHIP8_DISPLAY_HSIZE; h++) {
    for (int v = 0; v < CHIP8_DISPLAY_VSIZE / 8; v++) {
      // printf("%02x",display->display[h][v]);
      for (int p = 7; p >= 0; p--) {
        printf("%d", display->display[h][v] & (1 << p) ? 1 : 0);
      }
    }
    printf("\n");
  }
}

void chip8_load_rom(Chip8 *chip8, const uint8_t *rom, uint16_t size) {
  assert(size < CHIP8_MEM_SIZE - 0x200);
  memcpy(&chip8->memory[0x200], rom, size);
}

static inline uint16_t chip8_fetch(Chip8 *chip8) {
  uint16_t instruction = (chip8->memory[chip8->PC] << 8) + chip8->memory[chip8->PC + 1];
  chip8->PC += 2;
  return instruction;
}

// Execute a single instruction
void chip8_step(Chip8 *chip8) {
  printf("Instruction: %04x\n", chip8_fetch(chip8));
}

// I guess i will start implementing instructions and then figure out the decode
// logic 
// Instruction Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

/*
0nnn - SYS addr
Jump to a machine code routine at nnn.

This instruction is only used on the old computers on which Chip-8 was
originally implemented. It is ignored by modern interpreters.
*/
static inline void ins_sys_addr(Chip8 *chip8, uint16_t opcode) {}
