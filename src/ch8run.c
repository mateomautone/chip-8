#include <chip8.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

uint8_t uint8_rand();

int main(int argc, char const *argv[]) {
  chip8_t chip8;

  if (argc != 2) {
    printf("Usage: ch8run [ROMFILE]\n");
    return 1;
  } else {
    srand(time(NULL));
    // chip8_interface_t chip8_interface = {.rand = uint8_rand};
    // chip8_initialize(&chip8, chip8_interface);
    chip8_initialize(&chip8, (const chip8_interface_t){.rand = uint8_rand});
    if (chip8_load_rom_from_file(&chip8, argv[1]))
      return 1;
  }

  // chip8_load_rom(&chip8, (uint8_t[]){0xC1,0x99}, 2);
  for (long i = 0; i < 1100; i++) {
    chip8_step(&chip8);
  }

  return 0;
}

uint8_t uint8_rand() { return (uint8_t)rand(); }
