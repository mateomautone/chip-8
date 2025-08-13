#ifndef CHIP8
#define CHIP8

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

// Chip8 framebuffer (bit-packed version)
typedef uint8_t chip8_display_t[CHIP8_DISPLAY_HEIGHT][CHIP8_DISPLAY_WIDTH / 8];

// Chip8 external functions
typedef struct {
  uint8_t (*rand)(void);
  void (*draw_display)(const chip8_display_t *display, void *user_data);
  uint8_t display_update_flag; // Alternative to the callback, will just get set when necessary
  void *user_data; // So, this void pointer allows the backend to store stuff
} chip8_interface_t;

// Chip8 Structure
typedef struct {
  uint16_t PC;                      // Program Counter
  uint8_t SP;                       // Stack Pointer
  uint16_t stack[CHIP8_STACK_SIZE]; // Stack (should it be in memory(??))
  uint8_t V[16];                    // V Registers
  uint16_t I;                       // I 16bit Register
  uint8_t DT;                       // Sound Timer Register
  uint8_t ST;                       // Delay Timer register
  uint8_t keys[16];                 // Keys
  uint8_t memory[CHIP8_MEM_SIZE];   // 4K RAM
  chip8_display_t display;
  chip8_interface_t interface;
} chip8_t;

// Initialize CHIP8 struct
void chip8_initialize(chip8_t *chip8, const chip8_interface_t chip8_interface);

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
void chip8_print_registers(chip8_t *chip8, int flags);

// Print Display
void chip8_print_display(chip8_t *chip8, char on_char, char off_char);

// Hexdump memory region
void chip8_mem_hexdump(chip8_t *chip8, uint16_t start_addr, uint16_t end_addr);

// Execute an instruction
void chip8_step(chip8_t *chip8);

// Load ROM into the memory starting at address 0x200
void chip8_load_rom(chip8_t *chip8, const uint8_t *rom, uint16_t size);

// Load ROM from a file into memory starting at address 0x200
int chip8_load_rom_from_file(chip8_t *chip8, const char *filename);

// Set key
void chip8_set_key(chip8_t *chip8, uint8_t key);

// Reset key
void chip8_reset_key(chip8_t *chip8, uint8_t key);

// Tick Timers
void chip8_timer_tick(chip8_t *chip8);

#ifdef __cplusplus
}
#endif

#endif // CHIP8
