#include <chip8.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
  Chip8 chip8;
  chip8_initialize(&chip8);
  chip8_display_initialize(&chip8);
  // chip8_load_rom(&chip8, (unsigned char[]){0xde, 0xad}, 2);
  
  // Loading a ROM
  // FILE *fd = fopen("roms/2-ibm-logo.ch8", "rb");
  FILE *fd = fopen("roms/1-chip8-logo.ch8", "rb");
  // FILE *fd = fopen("roms/3-corax+.ch8", "rb");
  if (fd == NULL) {
    printf("No se pudo abrir\n");
    return -1;
  }
  fseek(fd, 0, SEEK_END);
  int filesize = ftell(fd);
  rewind(fd);
  fread(&chip8.memory[0x200], 1, filesize, fd);
  fclose(fd);

  for (long i = 0; i < 500; i++) {
    chip8_step(&chip8);
  }

  return 0;
}
