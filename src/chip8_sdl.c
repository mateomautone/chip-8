#include <SDL2/SDL.h>
#include <chip8.h>
#include <chip8_sdl.h>

int chip8_sdl_initialize(chip8_sdl_t *chip8_sdl, char *window_name,
                         int render_scale, SDL_Color background_color,
                         SDL_Color foreground_color) {
  // Init SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }
  // Create window
  chip8_sdl->window = SDL_CreateWindow(
      window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      CHIP8_DISPLAY_WIDTH * render_scale, CHIP8_DISPLAY_HEIGHT * render_scale,
      SDL_WINDOW_SHOWN);
  if (!chip8_sdl->window) {
    printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  // Create renderer
  chip8_sdl->renderer =
      SDL_CreateRenderer(chip8_sdl->window, -1, SDL_RENDERER_ACCELERATED);
  if (!chip8_sdl->renderer) {
    SDL_DestroyWindow(chip8_sdl->window);
    printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  // Set scale
  SDL_RenderSetScale(chip8_sdl->renderer, (float)render_scale,
                     (float)render_scale);
  // Set colors
  chip8_sdl->background_color = background_color;
  chip8_sdl->foreground_color = foreground_color;

  // Clear ?
  SDL_SetRenderDrawColor(chip8_sdl->renderer, chip8_sdl->background_color.r,
                         chip8_sdl->background_color.g,
                         chip8_sdl->background_color.b,
                         chip8_sdl->background_color.a);
  SDL_RenderClear(chip8_sdl->renderer);
  SDL_RenderPresent(chip8_sdl->renderer);

  return 0;
}

void chip8_sdl_destroy(chip8_sdl_t *chip8_sdl) {
  SDL_DestroyRenderer(chip8_sdl->renderer);
  SDL_DestroyWindow(chip8_sdl->window);
  SDL_Quit();
}

#ifndef NDEBUG
void chip8_sdl_test(chip8_sdl_t *chip8_sdl) {
  SDL_SetRenderDrawColor(chip8_sdl->renderer, chip8_sdl->background_color.r,
                         chip8_sdl->background_color.g,
                         chip8_sdl->background_color.b,
                         chip8_sdl->background_color.a);
  SDL_RenderClear(chip8_sdl->renderer);
  SDL_RenderPresent(chip8_sdl->renderer);
  SDL_Delay(1000);
  // printf("%d", chip8_sdl->foreground_color.r);
  SDL_SetRenderDrawColor(chip8_sdl->renderer, chip8_sdl->foreground_color.r,
                         chip8_sdl->foreground_color.g,
                         chip8_sdl->foreground_color.b,
                         chip8_sdl->foreground_color.a);
  SDL_RenderDrawPoint(chip8_sdl->renderer, CHIP8_DISPLAY_WIDTH / 2,
                      CHIP8_DISPLAY_HEIGHT / 2);
  SDL_RenderPresent(chip8_sdl->renderer);
  SDL_Delay(1000);
  SDL_SetRenderDrawColor(chip8_sdl->renderer, chip8_sdl->background_color.r,
                         chip8_sdl->background_color.g,
                         chip8_sdl->background_color.b,
                         chip8_sdl->background_color.a);
  SDL_RenderClear(chip8_sdl->renderer);
  SDL_RenderPresent(chip8_sdl->renderer);
  SDL_Delay(1000);
}
#endif

// This is probably slow af
void chip8_sdl_draw_display(const chip8_display_t *display, void *sdl_context) {
  chip8_sdl_t *chip8_sdl = (chip8_sdl_t*)sdl_context;
  SDL_SetRenderDrawColor(chip8_sdl->renderer, chip8_sdl->background_color.r,
                         chip8_sdl->background_color.g,
                         chip8_sdl->background_color.b,
                         chip8_sdl->background_color.a);
  SDL_RenderClear(chip8_sdl->renderer);
  SDL_SetRenderDrawColor(chip8_sdl->renderer, chip8_sdl->foreground_color.r,
                         chip8_sdl->foreground_color.g,
                         chip8_sdl->foreground_color.b,
                         chip8_sdl->foreground_color.a);
  for (int y = 0; y < CHIP8_DISPLAY_HEIGHT; y++) {
    for (int x = 0; x < CHIP8_DISPLAY_WIDTH / 8; x++) {
      for (int p = 7; p >= 0; p--) {
        if (((*display)[y][x] >> p) & 1) {
          SDL_RenderDrawPoint(chip8_sdl->renderer, x * 8 + 7 - p, y);
        }
      }
    }
  }
  SDL_RenderPresent(chip8_sdl->renderer);
}
