#include <chip8.h>
#include <ncurses.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
  Chip8 chip8;

  if (argc == 1) {
    printf("Usage: ch8run [OPTION]... [ROMFILE]\n");
    return 1;
  }else {
    chip8_initialize(&chip8);
    if (chip8_load_rom_from_file(&chip8, argv[argc-1]))
      return 1;
  }

  for (long i = 0; i < 1000; i++) {
    chip8_step(&chip8);
  }

  return 0;
}
