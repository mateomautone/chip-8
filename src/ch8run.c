#include <chip8.h>
#include <chip8_sdl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

uint8_t uint8_rand(void);
void print_usage(void);
int parse_color(const char *str, SDL_Color *color);

int main(int argc, char *argv[]) {
  // Some variables
  typedef enum { NONE, SDL } Backend;
  Backend backend = SDL;
  uint32_t cycles_per_frame = 20;
  uint32_t render_scale = 16;
  uint32_t target_fps = 60;
  SDL_Color fg_color = {255, 0x68, 0x0E, 255};
  SDL_Color bg_color = {255, 0xFF, 0x6E, 0x28};

  // Parse options
  int opt;
  while ((opt = getopt(argc, argv, "hc:b:f:s:F:G:")) != -1) {
    switch (opt) {
    case 'h':
      print_usage();
      return 0;
    case 'c':
      if (!(cycles_per_frame = (uint32_t)atoi(optarg))) {
        printf("Cannot be 0\n");
        return 1;
      }
      break;
    case 'b':
      if (strcasecmp(optarg, "none") == 0)
        backend = NONE;
      else if (strcasecmp(optarg, "sdl") == 0)
        backend = SDL;
      else {
        fprintf(stderr, "Unknown backend: %s\n", optarg);
        print_usage();
        return 1;
      }
      break;
    case 'f':
      if (!(target_fps = (uint32_t)atoi(optarg))) {
        printf("Cannot be 0\n");
        return 1;
      }
      break;
    case 's':
      if (!(render_scale = (uint32_t)atoi(optarg))) {
        printf("Cannot be 0\n");
        return 1;
      }
      break;
    case 'F':
      parse_color(optarg, &fg_color);
      break;
    case 'G':
      parse_color(optarg, &bg_color);
      break;
    default:
      print_usage();
      return 1;
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
    if (chip8_sdl_initialize(&chip8_sdl, argv[argc - 1], render_scale, bg_color,
                             fg_color))
      return 1;
#ifndef NDEBUG
    // chip8_sdl_test(&chip8_sdl);
#endif
  }

  // Setup chip8 interface
  srand((unsigned int)time(NULL)); // Seeding random number generator
  chip8_interface_t chip8_interface = {.rand = uint8_rand};
  if (backend == SDL) {
#ifdef CHIP8_USE_DRAW_CALLBACK
    chip8_interface.draw_display = chip8_sdl_draw_display;
    chip8_interface.user_data = &chip8_sdl;
#endif /* ifdef CHIP8_USE_DRAW_CALLBACK */
  }

  // Initialize chip8 core
  chip8_t chip8;
  chip8_initialize(&chip8, chip8_interface);
  if (chip8_load_rom_from_file(&chip8, argv[optind])) {
    return 1;
  }

  // Enter SDL Loop
  if (backend == SDL) {
    chip8_sdl_run(&chip8, &chip8_sdl, cycles_per_frame, target_fps);
  }

  // Clean up stuff
  if (backend == SDL) {
    chip8_sdl_destroy(&chip8_sdl);
  }
  return 0;
}

// Parses "R,G,B" into SDL_Color
int parse_color(const char *str, SDL_Color *color) {
  unsigned int r, g, b;
  if (sscanf(str, "%u,%u,%u", &r, &g, &b) != 3)
    return -1;
  color->r = (uint8_t)r;
  color->g = (uint8_t)g;
  color->b = (uint8_t)b;
  color->a = 255;
  return 0;
}

void print_usage(void) {
  printf("Usage: ch8run [OPTION]... [ROMFILE]\n\n");
  printf("Options:\n");
  printf("  -h         display this help\n");
  printf("  -c NUM     number of cycles per frame (default: 20)\n");
  printf("  -b BACKEND choose backend (none, SDL) (default: SDL)\n");
  printf("  -f FPS     target frames per second (default: 60)\n");
  printf("  -s SCALE   render scale (default: 16)\n");
  printf("  -F R,G,B   foreground color (default: 104,14,13)\n");
  printf("  -G R,G,B   background color (default: 255,110,40)\n");
}

uint8_t uint8_rand(void) { return (uint8_t)(rand() & 0xFF); }
