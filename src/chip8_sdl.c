#include "SDL_stdinc.h"
#include <SDL2/SDL.h>
#include <chip8.h>
#include <chip8_sdl.h>
#include <stdint.h>

int chip8_sdl_initialize(chip8_sdl_t *chip8_sdl, char *window_name,
                         uint32_t render_scale, SDL_Color background_color,
                         SDL_Color foreground_color) {
  // Init SDL
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }
  // Create window
  chip8_sdl->window = SDL_CreateWindow(
      window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      (int)(CHIP8_DISPLAY_WIDTH * render_scale), (int)(CHIP8_DISPLAY_HEIGHT * render_scale),
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
  chip8_sdl_t *chip8_sdl = (chip8_sdl_t *)sdl_context;
  SDL_SetRenderDrawColor(chip8_sdl->renderer, chip8_sdl->background_color.r,
                         chip8_sdl->background_color.g,
                         chip8_sdl->background_color.b,
                         chip8_sdl->background_color.a);
  SDL_RenderClear(chip8_sdl->renderer);
  SDL_SetRenderDrawColor(chip8_sdl->renderer, chip8_sdl->foreground_color.r,
                         chip8_sdl->foreground_color.g,
                         chip8_sdl->foreground_color.b,
                         chip8_sdl->foreground_color.a);
  for (int32_t y = 0; y < (int32_t)CHIP8_DISPLAY_HEIGHT; y++) {
    for (int32_t x = 0; x < (int32_t)CHIP8_DISPLAY_WIDTH / 8; x++) {
      for (int8_t p = 7; p >= 0; p--) {
        if (((*display)[y][x] >> p) & 1) {
          SDL_RenderDrawPoint(chip8_sdl->renderer, x * 8 + 7 - p, y);
        }
      }
    }
  }
  SDL_RenderPresent(chip8_sdl->renderer);
}

static inline uint8_t sdl_key_to_chip8_key(SDL_Keycode key) {
  switch (key) {
  case SDLK_1:
    return 0x1;
  case SDLK_2:
    return 0x2;
  case SDLK_3:
    return 0x3;
  case SDLK_4:
    return 0xC;
  case SDLK_q:
    return 0x4;
  case SDLK_w:
    return 0x5;
  case SDLK_e:
    return 0x6;
  case SDLK_r:
    return 0xD;
  case SDLK_a:
    return 0x7;
  case SDLK_s:
    return 0x8;
  case SDLK_d:
    return 0x9;
  case SDLK_f:
    return 0xE;
  case SDLK_z:
    return 0xA;
  case SDLK_x:
    return 0x0;
  case SDLK_c:
    return 0xB;
  case SDLK_v:
    return 0xF;
  default:
    return 0xFF; // I dont have a -1
  }
}

// Basic loop implementation
// TODO: Improve timings and stuf
void chip8_sdl_run(chip8_t *chip8, chip8_sdl_t *chip8_sdl,
                   uint32_t cycles_per_frame, uint32_t target_fps) {
  SDL_Event event;
  SDL_bool running = SDL_TRUE;

  while (running) {
    Uint32 start_ticks = SDL_GetTicks();
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = SDL_FALSE;
      } else if (event.type == SDL_KEYDOWN) {
        if (sdl_key_to_chip8_key(event.key.keysym.sym) != 0xFF) {
          chip8_set_key(chip8, sdl_key_to_chip8_key(event.key.keysym.sym));
        }
      } else if (event.type == SDL_KEYUP) {
        if (sdl_key_to_chip8_key(event.key.keysym.sym) != 0xFF) {
          chip8_reset_key(chip8, sdl_key_to_chip8_key(event.key.keysym.sym));
        }
      } else if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_EXPOSED ||
            event.window.event == SDL_WINDOWEVENT_RESTORED) {
          // Redraw when it gets minimized and stuff
          chip8_sdl_draw_display((const chip8_display_t *)&chip8->display,
                                 chip8_sdl);
        }
      }
    }
#ifndef CHIP8_USE_DRAWCALLBACK
    if (chip8->interface.display_update_flag)
      chip8_sdl_draw_display((const chip8_display_t *)&chip8->display, chip8_sdl);
#endif /* ifdef CHIP8_USE_DRAWCALLBACK */
#ifdef CHIP8_WAIT_VBLANK
    chip8->interface.vblank_ready = 1;
#endif /* ifdef CHIP8_WAIT_VBLANK */
    for (uint32_t i = 0; i < cycles_per_frame; i++) {
      chip8_step(chip8);
    }
    chip8_timer_tick(chip8);
    Uint32 end_ticks = SDL_GetTicks();
    SDL_Delay((1000 / target_fps) - (start_ticks - end_ticks));
  }

}

