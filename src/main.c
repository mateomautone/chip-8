#include <chip8.h>

int main(int argc, char const *argv[]) {
  Chip8 chip8;
  Chip8Display display;
  chip8_initialize(&chip8);
  chip8_display_initialize(&display);
  chip8_load_rom(&chip8, (unsigned char[]){0xde, 0xad}, 2);
  chip8_mem_hexdump(&chip8, 0, CHIP8_MEM_SIZE - 1);
  chip8_print_registers(&chip8);
  chip8_print_display(&display);

  /* code */
  return 0;
}
