#include <chip8.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
  CHIP8 chip8;
  chip8_initialize(&chip8);
  chip8_print_registers(&chip8);
  chip8_mem_hexdump(&chip8, 0, CHIP8_MEM_SIZE - 1);

  /* code */
  return 0;
}
