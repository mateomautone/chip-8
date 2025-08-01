#ifndef CHIP_8
#define CHIP_8

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
I will try to define all externally accesible functions and data-types for the
emulator/interpreter core in this file, my aim is to try to make this core
portable to other projects maybe
*/

// Defines
#define CHIP8_MEM_SIZE 4096

// I will imagine 'CHIP8' as if it were some sort of CPU (or kind of virtual cpu
// since it's really an interpreter(?))
typedef struct {
  uint16_t PC;                    // Program Counter
  uint8_t SP;                     // Stack Pointer
  uint16_t stack[16];             // Stack (should it be in memory(??))
  uint8_t memory[CHIP8_MEM_SIZE]; // 4K RAM
  uint8_t V[16];                  // V Registers
  uint16_t I;                     // I 16bit Register
  uint8_t DT, ST;                 // Delay Timer register, Sound Timer register
} CHIP8;

// Initialize CHIP8 struct
void chip8_initialize(CHIP8 *chip8);

// Print Registers
void chip8_print_registers(CHIP8 *chip8);

// Hexdump memory region
void chip8_mem_hexdump(CHIP8 *chip8, uint16_t start_addr, uint16_t end_addr);

// Execute an instruction
void chip8_step(CHIP8 *chip8);

// Load ROM into the memory starting at address 0x200
int chip8_load_rom(CHIP8 *chip8, uint8_t rom[3584]);

#ifdef __cplusplus
}
#endif

#endif // CHIP_8
