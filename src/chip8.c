#include <assert.h>
#include <chip8.h>
#include <stdio.h>

// Initialize the CHIP8 values
void chip8_initialize(CHIP8 *chip8) { *chip8 = (CHIP8){0}; }

// Print Registers
void chip8_print_registers(CHIP8 *chip8) {
  printf("Program Counter\t0x%04x\n", chip8->PC);
  printf("Stack Pointer\t0x%02x\n", chip8->SP);
  printf("Stack\t\t");
  for (int i = 0; i < 16; i++) {
    printf("0x%04x ", chip8->stack[i]);
  }
  printf("\nI Register\t0x%04x\n", chip8->I);
  for (int i = 0; i < 16; i++) {
    printf("V%d Register\t0x%02x\n", i, chip8->V[i]);
  }
  printf("Delay Timer\t0x%02x\n", chip8->DT);
  printf("Sound Timer\t0x%02x\n", chip8->ST);
}

// Hexdump memory region
void chip8_mem_hexdump(CHIP8 *chip8, uint16_t start_addr, uint16_t end_addr) {
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

// Execute a single instruction
void chip8_step(CHIP8 *chip8) { printf("Stepping test\n"); }

// I guess i will start implementing instructions and then figure out the decode
// logic 
// Instruction Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

/*
0nnn - SYS addr
Jump to a machine code routine at nnn.

This instruction is only used on the old computers on which Chip-8 was
originally implemented. It is ignored by modern interpreters.
*/
static inline void ins_sys_addr(CHIP8 *chip8, uint16_t opcode) {}
