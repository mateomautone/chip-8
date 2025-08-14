#include <assert.h>
#include <chip8.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
  memcpy(&chip8->memory[CHIP8_FONT_DATA_START], fontset, sizeof(fontset));
}

void chip8_initialize(chip8_t *chip8, const chip8_interface_t chip8_interface) {
  // Set everything to zero
  memset(chip8, 0, sizeof(*chip8));
  // Initializing interface
  chip8->interface = chip8_interface;
  if (!chip8->interface.rand)
    chip8->interface.rand = NULL;
#ifdef CHIP8_USE_DRAW_CALLBACK
  if (!chip8->interface.draw_display)
    chip8->interface.draw_display = NULL;
#endif /* ifdef CHIP8_USE_DRAW_CALLBACK */
  // Setting PC
  chip8->PC = 0x200;
  // Loading Font
  load_font(chip8);
}

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

void chip8_print_display(chip8_t *chip8, char on_char, char off_char) {
  for (uint8_t h = 0; h < CHIP8_DISPLAY_HEIGHT; h++) {
    for (uint8_t w = 0; w < CHIP8_DISPLAY_WIDTH / 8; w++) {
      for (int8_t p = 7; p >= 0; p--) {
        printf("%c", chip8->display[h][w] & (1 << p) ? on_char : off_char);
      }
    }
    printf("\n");
  }
}

void chip8_load_rom(chip8_t *chip8, const uint8_t *rom, uint16_t size) {
  assert(size < CHIP8_MEM_SIZE - 0x200);
  memcpy(&chip8->memory[0x200], rom, size);
}

int chip8_load_rom_from_file(chip8_t *chip8, const char *filename) {
  FILE *fd = fopen(filename, "rb");
  if (fd == NULL) {
    printf("Could not open file\n");
    return -1;
  }
  fseek(fd, 0, SEEK_END);
  size_t filesize = (size_t)ftell(fd);
  rewind(fd);
  if (filesize > CHIP8_MEM_SIZE) {
    printf("ROM is too big!\n");
    fclose(fd);
    return -1;
  }
  size_t bytes_read = fread(&chip8->memory[0x200], 1, filesize, fd);
  if (bytes_read != filesize)
    printf("Error: expected %zu bytes, got %zu\n", filesize, bytes_read);
  fclose(fd);
  return 0;
}

void chip8_set_key(chip8_t *chip8, uint8_t key) {
  assert(key < 16);
  chip8->keys[key] = 1;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_KEYS);
#endif
}

void chip8_reset_key(chip8_t *chip8, uint8_t key) {
  assert(key < 16);
  chip8->keys[key] = 0;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_KEYS);
#endif
}

void chip8_timer_tick(chip8_t *chip8) {
  if (chip8->DT)
    chip8->DT--;
  if (chip8->ST)
    chip8->ST--;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_DT | PRINT_ST);
#endif
}

// 0nnn - SYS addr
static inline void ins_sys_addr(chip8_t *chip8, uint16_t instruction) {
  (void)chip8;
  (void)instruction;
#ifndef NDEBUG
  printf("Uninmplemented Instruction: 0nnn\n");
#endif
}

// 00E0 - CLS
static inline void ins_cls(chip8_t *chip8) {
  memset(chip8->display, 0x0, sizeof(chip8->display));
#ifdef CHIP8_USE_DRAW_CALLBACK
  if (chip8->interface.draw_display)
    chip8->interface.draw_display((const chip8_display_t *)&chip8->display,
                                  chip8->interface.user_data);
#else
  chip8->interface.display_update_flag = 1;
#endif /* ifndef CHIP8_USE_DRAW_CALLBACK */
#ifndef NDEBUG
  chip8_print_display(chip8, '#', ' ');
#endif
}

// 00EE - RET
static inline void ins_ret(chip8_t *chip8) {
  assert(chip8->SP < (sizeof(chip8->stack) / sizeof(chip8->PC)) &&
         chip8->SP >= 0);
  chip8->PC = chip8->stack[chip8->SP];
  chip8->SP--;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_SP | PRINT_STACK | PRINT_PC);
#endif
}

// 1nnn - JP addr
static inline void ins_jp_addr(chip8_t *chip8, uint16_t instruction) {
  chip8->PC = instruction & 0x0FFF;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_PC);
#endif
}

// 2nnn - CALL addr
static inline void ins_call_addr(chip8_t *chip8, uint16_t instruction) {
  assert(chip8->SP < (sizeof(chip8->stack) / sizeof(chip8->PC)));
  chip8->SP++;
  chip8->stack[chip8->SP] = chip8->PC;
  chip8->PC = instruction & 0x0FFF;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_STACK | PRINT_SP | PRINT_PC);
#endif
}

// 3xkk - SE Vx, byte
static inline void ins_se_vx_byte(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t kk = (instruction & 0x00FF);
  if (chip8->V[x] == kk)
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 4xkk - SNE Vx, byte
static inline void ins_sne_vx_byte(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t kk = (instruction & 0x00FF);
  if (chip8->V[x] != kk)
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 5xy0 - SE Vx, Vy
static inline void ins_se_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  if (chip8->V[x] == chip8->V[y])
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 6xkk - LD Vx, byte
static inline void ins_ld_vx_byte(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t kk = (instruction & 0x00FF);
  chip8->V[x] = kk;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 7xkk - ADD Vx, byte
static inline void ins_add_vx_byte(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t kk = (instruction & 0x00FF);
  chip8->V[x] += kk;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xy0 - LD Vx, Vy
static inline void ins_ld_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  chip8->V[x] = chip8->V[y];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xy1 - OR Vx, Vy
static inline void ins_or_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  chip8->V[x] |= chip8->V[y];
#ifdef CHIP8_VF_RESET
  chip8->V[0xF] = 0;
#endif
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xy2 - AND Vx, Vy
static inline void ins_and_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  chip8->V[x] &= chip8->V[y];
#ifdef CHIP8_VF_RESET
  chip8->V[0xF] = 0;
#endif
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xy3 - XOR Vx, Vy
static inline void ins_xor_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  chip8->V[x] ^= chip8->V[y];
#ifdef CHIP8_VF_RESET
  chip8->V[0xF] = 0;
#endif
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xy4 - ADD Vx, Vy
static inline void ins_add_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  uint8_t set_vf = chip8->V[x] > UINT8_MAX - chip8->V[y];
  chip8->V[x] += chip8->V[y];
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xy5 - SUB Vx, Vy
static inline void ins_sub_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  uint8_t set_vf = chip8->V[x] >= chip8->V[y];
  chip8->V[x] -= chip8->V[y];
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xy6 - SHR Vx {, Vy}
static inline void ins_shr_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
#ifdef CHIP8_SHIFT_VX_ONLY
  uint8_t set_vf = chip8->V[x] & 0x1;
  chip8->V[x] >>= 1;
#else
  uint8_t y = (instruction & 0x00F0) >> 4;
  chip8->V[x] = chip8->V[y];
  uint8_t set_vf = chip8->V[x] & 0x1;
  chip8->V[x] >>= 1;
#endif
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xy7 - SUBN Vx, Vy
static inline void ins_subn_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  uint8_t set_vf = chip8->V[y] >= chip8->V[x];
  chip8->V[x] = chip8->V[y] - chip8->V[x];
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 8xyE - SHL Vx {, Vy}
static inline void ins_shl_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
#ifdef CHIP8_SHIFT_VX_ONLY
  uint8_t set_vf = (chip8->V[x] >> 7) & 0x1;
  chip8->V[x] <<= 1;
#else
  uint8_t y = (instruction & 0x00F0) >> 4;
  chip8->V[x] = chip8->V[y];
  uint8_t set_vf = (chip8->V[x] >> 7) & 0x1;
  chip8->V[x] <<= 1;
#endif
  chip8->V[0xF] = set_vf;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// 9xy0 - SNE Vx, Vy
static inline void ins_sne_vx_vy(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  if (chip8->V[x] != chip8->V[y])
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_PC);
#endif
}

// Annn - LD I, addr
static inline void ins_ld_i_addr(chip8_t *chip8, uint16_t instruction) {
  chip8->I = instruction & 0x0FFF;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_I);
#endif
}

// Bnnn - JP V0, addr
static inline void ins_jp_v0_addr(chip8_t *chip8, uint16_t instruction) {
  uint16_t addr = instruction & 0x0FFF;
#ifdef CHIP8_JUMP_USE_VX
  uint8_t x = (addr & 0x0F00) >> 8;
  chip8->PC = addr + chip8->V[x];
#else
  chip8->PC = addr + chip8->V[0];
#endif
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_PC);
#endif
}

// Cxkk - RND Vx, byte
static inline void ins_rnd_vx_byte(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t kk = (instruction & 0x00FF);
  if (chip8->interface.rand) {
    chip8->V[x] = chip8->interface.rand() & kk;
  } else {
    chip8->V[x] = 0x77 & kk; // A totally random number
  }
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// Dxyn - DRW Vx, Vy, nibble
#ifndef CHIP8_BUGGY
static inline void ins_drw_vx_vy(chip8_t *chip8, uint16_t instruction) {
#ifdef CHIP8_WAIT_VBLANK
  if (!chip8->interface.vblank_ready) {
    chip8->PC -= 2;
    return;
  } else {
    chip8->interface.vblank_ready = 0;
  }
#endif

  uint8_t x = (instruction & 0x0F00) >> 8;
  uint8_t y = (instruction & 0x00F0) >> 4;
  uint8_t n = (instruction & 0x000F);
  uint8_t Vx = chip8->V[x];
  uint8_t Vy = chip8->V[y];
  uint8_t xpos = Vx % CHIP8_DISPLAY_WIDTH;
  uint8_t ypos = Vy % CHIP8_DISPLAY_HEIGHT;

  assert(chip8->I < CHIP8_MEM_SIZE);
  assert(chip8->I + n < CHIP8_MEM_SIZE);

  chip8->V[0xF] = 0;
  uint8_t set_vf = 0;

  // Lets go row by row according to n
  for (uint8_t row = 0; row < n; row++) {
    uint8_t sprite_row = chip8->memory[chip8->I + row];
    // It could be the case that drawing will cross a boundary,
    // so I'll separate it into two bytes (xpos % 8)
    uint8_t sprite_row_first = sprite_row >> (xpos % 8u);
    uint8_t sprite_row_second = sprite_row << (8u - (xpos % 8u));
    if (ypos + row < CHIP8_DISPLAY_HEIGHT) { // It will CLIP on the bottom
      set_vf |=
          (chip8->display[ypos + row][xpos / 8u] & sprite_row_first) ? 1 : 0;
      chip8->display[ypos + row][xpos / 8u] ^= sprite_row_first;
      if ((xpos / 8u + 1u) < 8u) { // It will CLIP on right
        set_vf |=
            (chip8->display[ypos + row][xpos / 8u + 1u] & sprite_row_second)
                ? 1
                : 0;
        chip8->display[ypos + row][xpos / 8u + 1u] ^= sprite_row_second;
      }
    }
  }
  chip8->V[0xF] = set_vf;

#ifdef CHIP8_USE_DRAW_CALLBACK
  if (chip8->interface.draw_display)
    chip8->interface.draw_display((const chip8_display_t *)&chip8->display,
                                  chip8->interface.user_data);
#else
  chip8->interface.display_update_flag = 1;
#endif /* ifndef CHIP8_USE_DRAW_CALLBACK */
#ifndef NDEBUG
  chip8_print_display(chip8, '#', ' ');
#endif
}
#else // The first buggy implementation TBR
static inline void ins_drw_vx_vy(chip8_t *chip8, uint16_t instruction) {
#ifdef CHIP8_WAIT_VBLANK
  if (!chip8->interface.vblank_ready) {
    chip8->PC -= 2;
    return;
  } else {
    chip8->interface.vblank_ready = 0;
  }
#endif
  unsigned char x = chip8->V[(instruction & 0x0F00) >> 8] % CHIP8_DISPLAY_WIDTH;
  unsigned char y =
      chip8->V[(instruction & 0x00F0) >> 4] % CHIP8_DISPLAY_HEIGHT;
  unsigned char n = chip8->V[(instruction & 0x000F)];
  assert(chip8->I < CHIP8_MEM_SIZE);
  assert(chip8->I + n < CHIP8_MEM_SIZE);
  unsigned char pixel_erased = 0;
  for (unsigned char i = 0; i < (instruction & 0x000F); i++) {
    uint8_t spritebyte = chip8->memory[chip8->I + i];
    pixel_erased |= chip8->display[(y + i) % CHIP8_DISPLAY_HEIGHT][x / 8] &
                        spritebyte >> (x % 8) &&
                    chip8->display[(y + i) % CHIP8_DISPLAY_HEIGHT]
                                  [(x / 8 + 1) % (CHIP8_DISPLAY_WIDTH / 8)] &
                        spritebyte << (8 - (x % 8));
    chip8->display[(y + i) % CHIP8_DISPLAY_HEIGHT][(x / 8)] ^=
        (uint8_t)(spritebyte >> (x % 8));
    chip8->display[(y + i) % CHIP8_DISPLAY_HEIGHT]
                  [(x / 8 + 1) % (CHIP8_DISPLAY_WIDTH / 8)] ^=
        (uint8_t)(spritebyte << (8 - (x % 8)));
  }
  chip8->V[0xF] = pixel_erased;
#ifdef CHIP8_USE_DRAW_CALLBACK
  if (chip8->interface.draw_display)
    chip8->interface.draw_display((const chip8_display_t *)&chip8->display,
                                  chip8->interface.user_data);
#else
  chip8->interface.display_update_flag = 1;
#endif /* ifndef CHIP8_USE_DRAW_CALLBACK */
#ifndef NDEBUG
  chip8_print_display(chip8, '#', ' ');
#endif
}
#endif /* ifndef CHIP8_BUGGY */

// Ex9E - SKP Vx
static inline void ins_skp_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  assert(x < 16);
  if (chip8->keys[chip8->V[x] % 16])
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_PC | PRINT_KEYS);
#endif
}

// ExA1 - SKNP Vx
static inline void ins_sknp_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  assert(x < 16);
  if (!chip8->keys[chip8->V[x] % 16])
    chip8->PC += 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_PC | PRINT_KEYS);
#endif
}

// Fx07 - LD Vx, DT
static inline void ins_ld_vx_dt(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  chip8->V[x] = chip8->DT;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_DT);
#endif
}

#ifdef CHIP8_FX0A_RELEASE
void chip8_save_key(chip8_t *chip8) {
  memcpy(chip8->previous_keys, chip8->keys, sizeof(chip8->keys));
}
#endif /* ifdef CHIP8_FX0A_RELEASE */

// Fx0A - LD Vx, K
static inline void ins_ld_vx_k(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  for (uint8_t i = 0; i < 16; i++) {
#ifdef CHIP8_FX0A_RELEASE
    if (!chip8->keys[i] && chip8->previous_keys[i]) {
      chip8->V[x] = i;
      return;
    }
#else
    if (chip8->keys[i]) {
      chip8->V[x] = i;
      return;
    }
#endif /* ifdef CHIP8_FX0A_RELEASE */
  }
  chip8->PC -= 2;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V);
#endif
}

// Fx15 - LD DT, Vx
static inline void ins_ld_dt_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  chip8->DT = chip8->V[x];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_DT);
#endif
}

// Fx18 - LD ST, Vx
static inline void ins_ld_st_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  chip8->ST = chip8->V[x];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_ST);
#endif
}

// Fx1E - ADD I, Vx
static inline void ins_add_i_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  chip8->I += chip8->V[x];
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
#endif
}

// Fx29 - LD F, Vx
static inline void ins_ld_f_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  chip8->I = (uint16_t)(CHIP8_FONT_DATA_START + 5 * chip8->V[x]);
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
#endif
}

// Fx33 - LD B, Vx
static inline void ins_ld_b_vx(chip8_t *chip8, uint16_t instruction) {
  assert(chip8->I + 2u < CHIP8_MEM_SIZE);
  uint8_t x = (instruction & 0x0F00) >> 8;
  chip8->memory[chip8->I] = (chip8->V[x] / 100) % 10;
  chip8->memory[chip8->I + 1] = (chip8->V[x] / 10) % 10;
  chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
#endif
}

// Fx55 - LD [I], Vx
static inline void ins_ld_i_vx(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  assert(chip8->I + x < CHIP8_MEM_SIZE);
  for (uint8_t i = 0; i <= x; i++) {
    chip8->memory[chip8->I + i] = chip8->V[i];
  }
#ifdef CHIP8_MEM_INCR
  chip8->I += x + 1; // increment I after storing registers
#endif
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
  chip8_mem_hexdump(chip8, chip8->I, chip8->I + 16);
#endif
}

// Fx65 - LD Vx, [I]
static inline void ins_ld_vx_i(chip8_t *chip8, uint16_t instruction) {
  uint8_t x = (instruction & 0x0F00) >> 8;
  assert(chip8->I + x < CHIP8_MEM_SIZE);
  for (uint8_t i = 0; i <= x; i++) {
    chip8->V[i] = chip8->memory[chip8->I + i];
  }
#ifdef CHIP8_MEM_INCR
  chip8->I += x + 1; // increment I after loading registers
#endif
#ifndef NDEBUG
  chip8_print_registers(chip8, PRINT_V | PRINT_I);
  chip8_mem_hexdump(chip8, chip8->I, chip8->I + 16);
#endif
}

// Fetch instruction and increase Program Counter by 2
static inline uint16_t chip8_fetch(chip8_t *chip8) {
  uint16_t instruction = (uint16_t)((chip8->memory[chip8->PC] << 8) +
                                    chip8->memory[chip8->PC + 1]);
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
      case 0x009E:
        ins_skp_vx(chip8, instruction);
        break;
      case 0x00A1:
        ins_sknp_vx(chip8, instruction);
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
