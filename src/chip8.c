#include <assert.h>
#include <chip8.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Load font data (050-09F apparent convention)
static inline void load_font(chip8_t *chip8) {
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
  memcpy(&chip8->memory[CHIP8_FONT_DATA_START], fontset, sizeof(fontset));
}

// Initialize the CHIP8 values
void chip8_initialize(chip8_t *chip8, const chip8_interface_t chip8_interface) {
  // *chip8 = (Chip8){0};
  // Initializing Registers
  memset(chip8->V, 0x00, sizeof(chip8->V));
  chip8->PC = 0x200;
  chip8->SP = 0x00;
  chip8->I = 0x0000;
  chip8->DT = 0x00;
  chip8->ST = 0x00;
  // Initializing Key States
  memset(chip8->keys, 0x00, sizeof(chip8->keys));
  // Initializing interface
  chip8->interface = chip8_interface;
  if (!chip8->interface.rand)
    chip8->interface.rand = NULL;
  if (!chip8->interface.draw_display)
    chip8->interface.draw_display = NULL;
  // Initializing stack and memory
  memset(chip8->stack, 0x00, sizeof(chip8->stack));
  memset(chip8->memory, 0x00, sizeof(chip8->memory));
  load_font(chip8);
  // Initializing display
  memset(chip8->display, 0, sizeof(chip8->display));
}

// Display Initialization
// void chip8_display_initialize(Chip8 *chip8) {
// memset(chip8->display, 0, sizeof(chip8->display));
// }

// Print Registers
void chip8_print_registers(chip8_t *chip8, int flags) {
  if (flags & PRINT_PC)
    printf("Program Counter\t0x%04x\n", chip8->PC);
  if (flags & PRINT_SP)
    printf("Stack Pointer\t0x%02x\n", chip8->SP);
  if (flags & PRINT_STACK) {
    printf("Stack\t\t");
    for (int i = 0; i < 16; i++) {
      printf("0x%04x ", chip8->stack[i]);
    }
    printf("\n");
  }
  if (flags & PRINT_I)
    printf("I Register\t0x%04x\n", chip8->I);
  if (flags & PRINT_V) {
    for (int i = 0; i < 16; i++) {
      printf("V%X Register\t0x%02x\n", i, chip8->V[i]);
    }
  }
  if (flags & PRINT_DT)
    printf("Delay Timer\t0x%02x\n", chip8->DT);
  if (flags & PRINT_ST)
    printf("Sound Timer\t0x%02x\n", chip8->ST);
  if (flags & PRINT_KEYS) {
    for (int i = 0; i < 16; i++) {
      printf("Key %X\t0x%02x\n", i, chip8->keys[i]);
    }
  }
}

// Hexdump memory region
void chip8_mem_hexdump(chip8_t *chip8, uint16_t start_addr, uint16_t end_addr) {
  assert(start_addr <= end_addr);
  assert(end_addr < CHIP8_MEM_SIZE);
  const int bytes_per_line = 32;
  for (int newline_counter = 0; start_addr <= end_addr; start_addr++) {
    if (newline_counter == bytes_per_line) {
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
void chip8_print_display(chip8_t *chip8, char on_char, char off_char) {
  for (int h = 0; h < CHIP8_DISPLAY_HEIGHT; h++) {
    for (int w = 0; w < CHIP8_DISPLAY_WIDTH / 8; w++) {
      for (int p = 7; p >= 0; p--) {
        printf("%c", chip8->display[h][w] & (1 << p) ? on_char : off_char);
      }
    }
    printf("\n");
  }
}

// Load a ROM from memory
void chip8_load_rom(chip8_t *chip8, const uint8_t *rom, uint16_t size) {
  assert(size < CHIP8_MEM_SIZE - 0x200);
  memcpy(&chip8->memory[0x200], rom, size);
}

// Load a ROM from file
int chip8_load_rom_from_file(chip8_t *chip8, const char *filename) {
  FILE *fd = fopen(filename, "rb");
  if (fd == NULL) {
    printf("Could not open file\n");
    return -1;
  }
  fseek(fd, 0, SEEK_END);
  int filesize = ftell(fd);
  rewind(fd);
  if (filesize > CHIP8_MEM_SIZE) {
    printf("ROM is too big!\n");
    fclose(fd);
    return -1;
  }
  fread(&chip8->memory[0x200], 1, filesize, fd);
  fclose(fd);
  return 0;
}

/*
Instruction Reference: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

nnn or addr - A 12-bit value, the lowest 12 bits of the instruction
n or nibble - A 4-bit value, the lowest 4 bits of the instruction
x - A 4-bit value, the lower 4 bits of the high byte of the instruction
y - A 4-bit value, the upper 4 bits of the low byte of the instruction
kk or byte - An 8-bit value, the lowest 8 bits of the instruction
*/

/*
0nnn - SYS addr
Jump to a machine code routine at nnn.

This instruction is only used on the old computers on which Chip-8 was
originally implemented. It is ignored by modern interpreters.
*/
static inline void ins_sys_addr(chip8_t *chip8, uint16_t instruction) {
#ifndef NDEBUG
  printf("Uninmplemented Instruction: 0nnn\n");
#endif
}

/*
00E0 - CLS
Clear the display.
*/
static inline void ins_cls(chip8_t *chip8) {
  memset(chip8->display, 0x0, sizeof(chip8->display));
  if (chip8->interface.draw_display)
    chip8->interface.draw_display(&chip8->display);
#ifndef NDEBUG
  chip8_print_display(chip8, '#', ' ');
#endif
}

/*
00EE - RET
Return from a subroutine.

The interpreter sets the program counter to the address at the top of the stack,
then subtracts 1 from the stack pointer.
*/
static inline void ins_ret(chip8_t *chip8) {
  assert(chip8->SP < (sizeof(chip8->stack) / sizeof(chip8->PC)) &&
         chip8->SP >= 0);
  chip8->PC = chip8->stack[chip8->SP];
  //% (sizeof(chip8->stack) / sizeof(chip8->PC))];
  chip8->SP--;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_SP | PRINT_STACK | PRINT_PC);
#endif
}

/*
1nnn - JP addr
Jump to location nnn.

The interpreter sets the program counter to nnn.
*/
static inline void ins_jp_addr(chip8_t *chip8, uint16_t instruction) {
  chip8->PC = instruction & 0x0FFF;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_PC);
#endif
}

/*
2nnn - CALL addr
Call subroutine at nnn.

The interpreter increments the stack pointer, then puts the current PC on the
top of the stack. The PC is then set to nnn.
*/
static inline void ins_call_addr(chip8_t *chip8, uint16_t instruction) {
  assert(chip8->SP < (sizeof(chip8->stack) / sizeof(chip8->PC)));
  chip8->SP++;
  chip8->stack[chip8->SP] = chip8->PC;
  chip8->PC = instruction & 0x0FFF;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_STACK | PRINT_SP | PRINT_PC);
#endif
}

/*
3xkk - SE Vx, byte
Skip next instruction if Vx = kk.

The interpreter compares register Vx to kk, and if they are equal, increments
the program counter by 2.
*/
static inline void ins_se_vx_byte(chip8_t *chip8, uint16_t instruction) {
  if (chip8->V[(instruction & 0x0F00) >> 8] == (instruction & 0x00FF))
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
4xkk - SNE Vx, byte
Skip next instruction if Vx != kk.

The interpreter compares register Vx to kk, and if they are not equal,
increments the program counter by 2.
*/
static inline void ins_sne_vx_byte(chip8_t *chip8, uint16_t instruction) {
  if (chip8->V[(instruction & 0x0F00) >> 8] != (instruction & 0x00FF))
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
5xy0 - SE Vx, Vy
Skip next instruction if Vx = Vy.

The interpreter compares register Vx to register Vy, and if they are equal,
increments the program counter by 2.
*/
static inline void ins_se_vx_vy(chip8_t *chip8, uint16_t instruction) {
  if (chip8->V[(instruction & 0x0F00) >> 8] ==
      chip8->V[(instruction & 0x00F0) >> 4])
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
6xkk - LD Vx, byte
Set Vx = kk.

The interpreter puts the value kk into register Vx.
*/
static inline void ins_ld_vx_byte(chip8_t *chip8, uint16_t instruction) {
  chip8->V[(instruction & 0x0F00) >> 8] = instruction & 0x00FF;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
7xkk - ADD Vx, byte
Set Vx = Vx + kk.

Adds the value kk to the value of register Vx, then stores the result in Vx.
*/
static inline void ins_add_vx_byte(chip8_t *chip8, uint16_t instruction) {
  chip8->V[(instruction & 0x0F00) >> 8] += instruction & 0x00FF;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xy0 - LD Vx, Vy
Set Vx = Vy.

Stores the value of register Vy in register Vx.
*/
static inline void ins_ld_vx_vy(chip8_t *chip8, uint16_t instruction) {
  chip8->V[(instruction & 0x0F00) >> 8] = chip8->V[(instruction & 0x00F0) >> 4];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xy1 - OR Vx, Vy
Set Vx = Vx OR Vy.

Performs a bitwise OR on the values of Vx and Vy, then stores the result in Vx.
A bitwise OR compares the corrseponding bits from two values, and if either bit
is 1, then the same bit in the result is also 1. Otherwise, it is 0.
*/
static inline void ins_or_vx_vy(chip8_t *chip8, uint16_t instruction) {
  chip8->V[(instruction & 0x0F00) >> 8] |=
      chip8->V[(instruction & 0x00F0) >> 4];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xy2 - AND Vx, Vy
Set Vx = Vx AND Vy.

Performs a bitwise AND on the values of Vx and Vy, then stores the result in Vx.
A bitwise AND compares the corrseponding bits from two values, and if both bits
are 1, then the same bit in the result is also 1. Otherwise, it is 0.
*/
static inline void ins_and_vx_vy(chip8_t *chip8, uint16_t instruction) {
  chip8->V[(instruction & 0x0F00) >> 8] &=
      chip8->V[(instruction & 0x00F0) >> 4];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xy3 - XOR Vx, Vy
Set Vx = Vx XOR Vy.

Performs a bitwise exclusive OR on the values of Vx and Vy, then stores the
result in Vx. An exclusive OR compares the corrseponding bits from two values,
and if the bits are not both the same, then the corresponding bit in the result
is set to 1. Otherwise, it is 0.
*/
static inline void ins_xor_vx_vy(chip8_t *chip8, uint16_t instruction) {
  chip8->V[(instruction & 0x0F00) >> 8] ^=
      chip8->V[(instruction & 0x00F0) >> 4];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xy4 - ADD Vx, Vy
Set Vx = Vx + Vy, set VF = carry.

The values of Vx and Vy are added together. If the result is greater than 8 bits
(i.e., > 255,) VF is set to 1, otherwise 0. Only the lowest 8 bits of the result
are kept, and stored in Vx.
*/
static inline void ins_add_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t set_vf = chip8->V[(instruction & 0x0F00) >> 8] >
                  UINT8_MAX - chip8->V[(instruction & 0x00F0) >> 4] ? 1 : 0;
  chip8->V[(instruction & 0x0F00) >> 8] +=
      chip8->V[(instruction & 0x00F0) >> 4];
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xy5 - SUB Vx, Vy
Set Vx = Vx - Vy, set VF = NOT borrow.

If Vx > Vy, then VF is set to 1, otherwise 0. Then Vy is subtracted from Vx, and
the results stored in Vx.
*/
static inline void ins_sub_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t set_vf = chip8->V[(instruction & 0x0F00) >> 8] >
                  chip8->V[(instruction & 0x00F0) >> 4] ? 1 : 0;
  chip8->V[(instruction & 0x0F00) >> 8] -=
      chip8->V[(instruction & 0x00F0) >> 4];
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xy6 - SHR Vx {, Vy}
Set Vx = Vx SHR 1.

If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then
Vx is divided by 2.
*/
static inline void ins_shr_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t set_vf = chip8->V[(instruction & 0x0F00) >> 8] & 0x1;
  chip8->V[(instruction & 0x0F00) >> 8] >>= 1;
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xy7 - SUBN Vx, Vy
Set Vx = Vy - Vx, set VF = NOT borrow.

If Vy > Vx, then VF is set to 1, otherwise 0. Then Vx is subtracted from Vy, and
the results stored in Vx.
*/
static inline void ins_subn_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t set_vf = chip8->V[(instruction & 0x00F0) >> 4] >
                  chip8->V[(instruction & 0x0F00) >> 8] ? 1 : 0;
  chip8->V[(instruction & 0x0F00) >> 8] =
      chip8->V[(instruction & 0x00F0) >> 4] -
      chip8->V[(instruction & 0x0F00) >> 8];
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
8xyE - SHL Vx {, Vy}
Set Vx = Vx SHL 1.

If the most-significant bit of Vx is 1, then VF is set to 1, otherwise to 0.
Then Vx is multiplied by 2.
*/
static inline void ins_shl_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t set_vf = (chip8->V[(instruction & 0x0F00) >> 8] >> 7) & 0x1;
  chip8->V[(instruction & 0x0F00) >> 8] <<= 1;
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
9xy0 - SNE Vx, Vy
Skip next instruction if Vx != Vy.

The values of Vx and Vy are compared, and if they are not equal, the program
counter is increased by 2.
*/
static inline void ins_sne_vx_vy(chip8_t *chip8, uint16_t instruction) {
  if (chip8->V[(instruction & 0x0F00) >> 8] !=
      chip8->V[(instruction & 0x00F0) >> 4])
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_PC);
#endif
}

/*
Annn - LD I, addr
Set I = nnn.

The value of register I is set to nnn.
*/
static inline void ins_ld_i_addr(chip8_t *chip8, uint16_t instruction) {
  chip8->I = instruction & 0x0FFF;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_I);
#endif
}

/*
Bnnn - JP V0, addr
Jump to location nnn + V0.

The program counter is set to nnn plus the value of V0.
*/
static inline void ins_jp_v0_addr(chip8_t *chip8, uint16_t instruction) {
  chip8->PC = (instruction & 0x0FFF) + chip8->V[0];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_PC);
#endif
}

/*
Cxkk - RND Vx, byte
Set Vx = random byte AND kk.

The interpreter generates a random number from 0 to 255, which is then ANDed
with the value kk. The results are stored in Vx. See instruction 8xy2 for more
information on AND.
*/
static inline void ins_rnd_vx_byte(chip8_t *chip8, uint16_t instruction) {
  if (chip8->interface.rand) {
    chip8->V[(instruction & 0x0F00) >> 8] =
        chip8->interface.rand() & (instruction & 0x00FF);
  } else {
    chip8->V[(instruction & 0x0F00) >> 8] =
        0x77 & (instruction & 0x00FF); // A totally random number
  }
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
Dxyn - DRW Vx, Vy, nibble
Display n-byte sprite starting at memory location I at (Vx, Vy), set VF =
collision.

The interpreter reads n bytes from memory, starting at the address stored in I.
These bytes are then displayed as sprites on screen at coordinates (Vx, Vy).
Sprites are XORed onto the existing screen. If this causes any pixels to be
erased, VF is set to 1, otherwise it is set to 0. If the sprite is positioned so
part of it is outside the coordinates of the display, it wraps around to the
opposite side of the screen. See instruction 8xy3 for more information on XOR,
and section 2.4, Display, for more information on the Chip-8 screen and sprites.
*/
static inline void ins_drw_vx_vy(chip8_t *chip8, uint16_t instruction) {
  unsigned char x = chip8->V[(instruction & 0x0F00) >> 8] % CHIP8_DISPLAY_WIDTH;
  unsigned char y =
      chip8->V[(instruction & 0x00F0) >> 4] % CHIP8_DISPLAY_HEIGHT;
  assert(chip8->I < CHIP8_MEM_SIZE);
  assert(chip8->I + (instruction & 0x000F) < CHIP8_MEM_SIZE);
  unsigned char pixel_erased = 0;
  for (unsigned char i = 0; i < (instruction & 0x000F); i++) {
    uint8_t spritebyte = chip8->memory[chip8->I + i];
    pixel_erased |= chip8->display[(y + i) % CHIP8_DISPLAY_HEIGHT][x / 8] &
                        spritebyte >> (x % 8) &&
                    chip8->display[(y + i) % CHIP8_DISPLAY_HEIGHT]
                                  [(x / 8 + 1) % (CHIP8_DISPLAY_WIDTH / 8)] &
                        spritebyte << (8 - (x % 8));
    chip8->display[(y + i) % CHIP8_DISPLAY_HEIGHT][(x / 8)] ^=
        spritebyte >> (x % 8);
    chip8->display[(y + i) % CHIP8_DISPLAY_HEIGHT]
                  [(x / 8 + 1) % (CHIP8_DISPLAY_WIDTH / 8)] ^= spritebyte
                                                               << (8 - (x % 8));
  }
  chip8->V[0xF] = pixel_erased;
  if (chip8->interface.draw_display)
    chip8->interface.draw_display(&chip8->display);
  #ifndef NDEBUG
  chip8_print_display(chip8, '#', ' ');
  #endif
}

/*
Ex9E - SKP Vx
Skip next instruction if key with the value of Vx is pressed.

Checks the keyboard, and if the key corresponding to the value of Vx is
currently in the down position, PC is increased by 2.
*/
static inline void ins_skp_vx(chip8_t *chip8, uint16_t instruction) {
  if (chip8->keys[chip8->V[(instruction & 0x0F00) >> 8]])
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_PC | PRINT_KEYS);
#endif
}

/*
Fx07 - LD Vx, DT
Set Vx = delay timer value.

The value of DT is placed into Vx.
*/
static inline void ins_ld_vx_dt(chip8_t *chip8, uint16_t instruction) {
  chip8->V[(instruction & 0x0F00) >> 8] = chip8->DT;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_DT);
#endif
}

/*
Fx0A - LD Vx, K
Wait for a key press, store the value of the key in Vx.

All execution stops until a key is pressed, then the value of that key is stored
in Vx.
*/
static inline void ins_ld_vx_k(chip8_t *chip8, uint16_t instruction) {
  // TODO
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

/*
Fx15 - LD DT, Vx
Set delay timer = Vx.

DT is set equal to the value of Vx.
*/
static inline void ins_ld_dt_vx(chip8_t *chip8, uint16_t instruction) {
  chip8->DT = chip8->V[(instruction & 0x0F00) >> 8];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_DT);
#endif
}

/*
Fx18 - LD ST, Vx
Set sound timer = Vx.

ST is set equal to the value of Vx.
*/
static inline void ins_ld_st_vx(chip8_t *chip8, uint16_t instruction) {
  chip8->ST = chip8->V[(instruction & 0x0F00) >> 8];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_ST);
#endif
}

/*
Fx1E - ADD I, Vx
Set I = I + Vx.

The values of I and Vx are added, and the results are stored in I.
*/
static inline void ins_add_i_vx(chip8_t *chip8, uint16_t instruction) {
  chip8->I += chip8->V[(instruction & 0x0F00) >> 8];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
#endif
}

/*
Fx29 - LD F, Vx
Set I = location of sprite for digit Vx.

The value of I is set to the location for the hexadecimal sprite corresponding
to the value of Vx. See section 2.4, Display, for more information on the Chip-8
hexadecimal font.
*/
static inline void ins_ld_f_vx(chip8_t *chip8, uint16_t instruction) {
  chip8->I = CHIP8_FONT_DATA_START + 5 * chip8->V[(instruction & 0x0F00) >> 8];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
#endif
}

/*
Fx33 - LD B, Vx
Store BCD representation of Vx in memory locations I, I+1, and I+2.

The interpreter takes the decimal value of Vx, and places the hundreds digit in
memory at location in I, the tens digit at location I+1, and the ones digit at
location I+2.
*/
static inline void ins_ld_b_vx(chip8_t *chip8, uint16_t instruction) {
  assert(chip8->I + 2 < CHIP8_MEM_SIZE);
  chip8->memory[chip8->I] = (chip8->V[(instruction & 0x0F00) >> 8] / 100) % 10;
  chip8->memory[chip8->I + 1] =
      (chip8->V[(instruction & 0x0F00) >> 8] / 10) % 10;
  chip8->memory[chip8->I + 2] = chip8->V[(instruction & 0x0F00) >> 8] % 10;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
#endif
}

/*
Fx55 - LD [I], Vx
Store registers V0 through Vx in memory starting at location I.

The interpreter copies the values of registers V0 through Vx into memory,
starting at the address in I.
*/
static inline void ins_ld_i_vx(chip8_t *chip8, uint16_t instruction) {
  assert(chip8->I + ((instruction & 0x0F00) >> 8) < CHIP8_MEM_SIZE);
  for (uint8_t i = 0; i <= ((instruction & 0x0F00) >> 8); i++) {
    chip8->memory[chip8->I + i] = chip8->V[i];
  }
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
  chip8_mem_hexdump(chip8, chip8->I, chip8->I + 16);
#endif
}

/*
Fx65 - LD Vx, [I]
Read registers V0 through Vx from memory starting at location I.

The interpreter reads values from memory starting at location I into registers
V0 through Vx.
*/
static inline void ins_ld_vx_i(chip8_t *chip8, uint16_t instruction) {
  assert(chip8->I + ((instruction & 0x0F00) >> 8) < CHIP8_MEM_SIZE);
  for (uint8_t i = 0; i <= ((instruction & 0x0F00) >> 8); i++) {
    chip8->V[i] = chip8->memory[chip8->I + i];
  }
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
  chip8_mem_hexdump(chip8, chip8->I, chip8->I + 16);
#endif
}

// Fetch and instruction and increase Program Counter by 2
static inline uint16_t chip8_fetch(chip8_t *chip8) {
  uint16_t instruction =
      (chip8->memory[chip8->PC] << 8) + chip8->memory[chip8->PC + 1];
  chip8->PC += 2;
#ifndef NDEBUG
  printf("Fetching Instruction: %04x\n", instruction);
#endif
  return instruction;
}

// Decode and execute an instruction
static inline void chip8_decode_execute(chip8_t *chip8, uint16_t instruction) {
  switch (instruction) {
  case 0x00E0:
    ins_cls(chip8);
    break;
  case 0x00EE:
    ins_ret(chip8);
    break;
  default:
    switch (instruction & 0xF000) {
    case 0x0000:
      ins_sys_addr(chip8, instruction);
      break;
    case 0x1000:
      ins_jp_addr(chip8, instruction);
      break;
    case 0x2000:
      ins_call_addr(chip8, instruction);
      break;
    case 0x3000:
      ins_se_vx_byte(chip8, instruction);
      break;
    case 0x4000:
      ins_sne_vx_byte(chip8, instruction);
      break;
    case 0x5000:
      switch (instruction & 0x000F) {
      case 0x0000:
        ins_se_vx_vy(chip8, instruction);
        break;
      default:
        printf("Unknown Instruction found: %04x\n", instruction);
      }
      break;
    case 0x6000:
      ins_ld_vx_byte(chip8, instruction);
      break;
    case 0x7000:
      ins_add_vx_byte(chip8, instruction);
      break;
    case 0x8000:
      switch (instruction & 0x000F) {
      case 0x0000:
        ins_ld_vx_vy(chip8, instruction);
        break;
      case 0x0001:
        ins_or_vx_vy(chip8, instruction);
        break;
      case 0x0002:
        ins_and_vx_vy(chip8, instruction);
        break;
      case 0x0003:
        ins_xor_vx_vy(chip8, instruction);
        break;
      case 0x0004:
        ins_add_vx_vy(chip8, instruction);
        break;
      case 0x0005:
        ins_sub_vx_vy(chip8, instruction);
        break;
      case 0x0006:
        ins_shr_vx(chip8, instruction);
        break;
      case 0x0007:
        ins_subn_vx_vy(chip8, instruction);
        break;
      case 0x000E:
        ins_shl_vx(chip8, instruction);
        break;
      default:
        printf("Unknown Instruction found: %04x\n", instruction);
      }
      break;
    case 0x9000:
      switch (instruction & 0x000F) {
      case 0x0000:
        ins_sne_vx_vy(chip8, instruction);
        break;
      default:
        printf("Unknown Instruction found: %04x\n", instruction);
      }
      break;
    case 0xA000:
      ins_ld_i_addr(chip8, instruction);
      break;
    case 0xB000:
      ins_jp_v0_addr(chip8, instruction);
      break;
    case 0xC000:
      ins_rnd_vx_byte(chip8, instruction);
      break;
    case 0xD000:
      ins_drw_vx_vy(chip8, instruction);
      break;
    case 0xE000:
      switch (instruction & 0x00FF) {
      case 0x00A1:
        ins_skp_vx(chip8, instruction);
        break;
      default:
        printf("Unknown Instruction found: %04x\n", instruction);
      }
      break;
    case 0xF000:
      switch (instruction & 0x00FF) {
      case 0x0007:
        ins_ld_vx_dt(chip8, instruction);
        break;
      case 0x000A:
        ins_ld_vx_k(chip8, instruction);
        break;
      case 0x0015:
        ins_ld_dt_vx(chip8, instruction);
        break;
      case 0x0018:
        ins_ld_st_vx(chip8, instruction);
        break;
      case 0x001E:
        ins_add_i_vx(chip8, instruction);
        break;
      case 0x0029:
        ins_ld_f_vx(chip8, instruction);
        break;
      case 0x0033:
        ins_ld_b_vx(chip8, instruction);
        break;
      case 0x0055:
        ins_ld_i_vx(chip8, instruction);
        break;
      case 0x0065:
        ins_ld_vx_i(chip8, instruction);
        break;
      default:
        printf("Unknown Instruction found: %04x\n", instruction);
      }
      break;
    default:
      printf("Unknown Instruction found: %04x\n", instruction);
    }
  }
}

// Execute a single instruction
void chip8_step(chip8_t *chip8) {
  chip8_decode_execute(chip8, chip8_fetch(chip8));
}

#ifdef __cplusplus
}
#endif
