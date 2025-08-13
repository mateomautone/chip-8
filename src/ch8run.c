#include <chip8.h>
#include <chip8_sdl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

uint8_t uint8_rand(void);
void print_usage(void);

int main(int argc, char *argv[]) {
  // Some variables
  typedef enum { NONE, SDL } Backend;
  Backend backend = SDL;
  int cycles_per_frame = 20;
  int render_scale = 16;

  // Parse options
  int opt;
  while ((opt = getopt(argc, argv, "hc:b:")) != -1) {
    switch (opt) {
    case 'h':
      print_usage();
      return 0;
    case 'c':
      // srand((unsigned int)time(NULL));
      // chip8_initialize(&chip8, (const chip8_interface_t){.rand =
      // uint8_rand}); if (optind != argc - 1) { print_usage(); return 1;
      // }
      // if (chip8_load_rom_from_file(&chip8,
      // optind == argc - 1 ? argv[optind] : NULL)) {
      // return 1;
      // }
      // int cycles = atoi(optarg);
      // printf("%d", atoi(optarg));
      // for (int i = 0; i < cycles; i++) {
      // chip8_step(&chip8);
      // }
      // return 0;
      if (!(cycles_per_frame = atoi(optarg))) {
        print_usage();
        return 1;
      }
      break;
    case 'b':
      if (strcasecmp(optarg, "none") == 0) {
        backend = NONE;
      } else if (strcasecmp(optarg, "sdl") == 0) {
        backend = SDL;
      } else {
        fprintf(stderr, "Unknown backend: %s\n", optarg);
        print_usage();
        return 1;
      }
      break;
    }
  }
  if (optind != argc - 1 || argc == 1) {
    printf("Error: Missing ROM file\n");
    print_usage();
    return 1;
  }

  // Initialize chip8_sdl
  chip8_sdl_t chip8_sdl;
  if (backend == SDL) {
    if (chip8_sdl_initialize(
            &chip8_sdl, argv[argc - 1], render_scale,
            (SDL_Color){.a = 255, .r = 0x68, .g = 0x0E, .b = 0x0D},
            (SDL_Color){.a = 255, .r = 0xFF, .g = 0x6E, .b = 0x28}))
      return 1;
#ifndef NDEBUG
    // chip8_sdl_test(&chip8_sdl);
#endif
  }

  // Setup chip8 interface
  srand((unsigned int)time(NULL)); // Seeding random number generator
  chip8_interface_t chip8_interface = {.rand = uint8_rand};
  if (backend == SDL) {
#ifdef CHIP8_USE_DRAWCALLBACK
    chip8_interface.draw_display = chip8_sdl_draw_display;
#endif /* ifdef CHIP8_USE_DRAWCALLBACK */
    chip8_interface.user_data = &chip8_sdl;
  }

  // Initialize chip8 core
  chip8_t chip8;
  chip8_initialize(&chip8, chip8_interface);
  if (chip8_load_rom_from_file(&chip8, argv[optind])) {
    return 1;
  }

  // Enter SDL Loop
  if (backend == SDL) {
    chip8_sdl_run(&chip8, &chip8_sdl, cycles_per_frame);
  }

  // Clean up stuff
  if (backend == SDL) {
    chip8_sdl_destroy(&chip8_sdl);
  }
  return 0;
}

void print_usage(void) {
  printf("Usage: ch8run [OPTION]... [ROMFILE]\n\n");
  printf("Options:\n");
  printf("  -h\tdisplay this help\n");
  printf("  -c\tnumber of cycles per frame (Default: 20)\n");
  printf("  -b\tchoose backend (none, SDL, ncurses) (Default: SDL)\n");
  printf("\n");
}

uint8_t uint8_rand(void) { return (uint8_t)rand(); }
