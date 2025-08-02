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
#define CHIP8_STACK_SIZE 16

// I will imagine 'CHIP8' as if it were some sort of CPU (or kind of virtual cpu
// since it's really an interpreter(?))
typedef struct {
  uint16_t PC;                    // Program Counter
  uint8_t SP;                     // Stack Pointer
  uint16_t stack[CHIP8_STACK_SIZE];             // Stack (should it be in memory(??))
  uint8_t V[16];                  // V Registers
  uint16_t I;                     // I 16bit Register
  uint8_t DT, ST;                 // Delay Timer register, Sound Timer register
  uint8_t memory[CHIP8_MEM_SIZE]; // 4K RAM
} Chip8;

#define CHIP8_DISPLAY_VSIZE 64
#define CHIP8_DISPLAY_HSIZE 32

typedef struct {
  uint8_t display[CHIP8_DISPLAY_HSIZE][CHIP8_DISPLAY_VSIZE / 8];
} Chip8Display;

// Initialize CHIP8 struct
void chip8_initialize(Chip8 *chip8);

// Initialize the display I guess
void chip8_display_initialize(Chip8Display *display);

// Print Registers
void chip8_print_registers(Chip8 *chip8);

// Print Display
void chip8_print_display(Chip8Display *display);

// Hexdump memory region
void chip8_mem_hexdump(Chip8 *chip8, uint16_t start_addr, uint16_t end_addr);

// Execute an instruction
void chip8_step(Chip8 *chip8);

// Load ROM into the memory starting at address 0x200
void chip8_load_rom(Chip8 *chip8, const uint8_t *rom, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif // CHIP_8
