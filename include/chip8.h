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
#define CHIP8_DISPLAY_WIDTH 64
#define CHIP8_DISPLAY_HEIGHT 32
#define CHIP8_FONT_DATA_START 0x50

// I will imagine 'CHIP8' as if it were some sort of CPU (or kind of virtual cpu
// since it's really an interpreter(?))

typedef struct {
  uint16_t PC;                      // Program Counter
  uint8_t SP;                       // Stack Pointer
  uint16_t stack[CHIP8_STACK_SIZE]; // Stack (should it be in memory(??))
  uint8_t V[16];                    // V Registers
  uint16_t I;                       // I 16bit Register
  uint8_t DT;                       // Sound Timer Register
  uint8_t ST;                       // Delay Timer register
  uint8_t memory[CHIP8_MEM_SIZE];   // 4K RAM
  uint8_t display[CHIP8_DISPLAY_HEIGHT][CHIP8_DISPLAY_WIDTH / 8];
  union keyboard {
    struct {
      unsigned char key_0 : 1;
      unsigned char key_1 : 1;
      unsigned char key_2 : 1;
      unsigned char key_3 : 1;
      unsigned char key_4 : 1;
      unsigned char key_5 : 1;
      unsigned char key_6 : 1;
      unsigned char key_7 : 1;
      unsigned char key_8 : 1;
      unsigned char key_9 : 1;
      unsigned char key_a : 1;
      unsigned char key_b : 1;
      unsigned char key_c : 1;
      unsigned char key_d : 1;
      unsigned char key_e : 1;
      unsigned char key_f : 1;
    } keys;
    uint16_t value;
  } keyboard;
} Chip8;

// Initialize CHIP8 struct
void chip8_initialize(Chip8 *chip8);

// Initialize the display I guess
void chip8_display_initialize(Chip8 *chip8);

// Print flags
#define PRINT_PC (1 << 0)
#define PRINT_SP (1 << 1)
#define PRINT_STACK (1 << 2)
#define PRINT_I (1 << 3)
#define PRINT_V (1 << 4)
#define PRINT_DT (1 << 5)
#define PRINT_ST (1 << 6)
#define PRINT_KEYS (1 << 7)

// Print Registers
void chip8_print_registers(Chip8 *chip8, int flags);

// Print Display
void chip8_print_display(Chip8 *chip8, char on_char, char off_char);

// Hexdump memory region
void chip8_mem_hexdump(Chip8 *chip8, uint16_t start_addr, uint16_t end_addr);

// Execute an instruction
void chip8_step(Chip8 *chip8);

// Load ROM into the memory starting at address 0x200
void chip8_load_rom(Chip8 *chip8, const uint8_t *rom, uint16_t size);

// Load ROM from a file into memory starting at address 0x200
int chip8_load_rom_from_file(Chip8 *chip8, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // CHIP_8
