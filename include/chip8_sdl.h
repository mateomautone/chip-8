#ifndef CHIP8_SDL
#define CHIP8_SDL

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <SDL2/SDL.h>
#include <chip8.h>

/*
So, here is the SDL backend for the chip8 emulator
TODO: add usage details
*/

// SDL struct type thing
typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Color background_color;
  SDL_Color foreground_color;
} chip8_sdl_t;

// Initialize SDL, returns 1 on error
int chip8_sdl_initialize(chip8_sdl_t *chip8_sdl, char *window_name, int render_scale, SDL_Color background_color, SDL_Color foreground_color);

// Destroy SDL
void chip8_sdl_destroy(chip8_sdl_t *chip8_sdl);

// Draw the chip8 display
void chip8_sdl_draw_display(const chip8_display_t *display, void *sdl_context);

#ifndef NDEBUG
void chip8_sdl_test(chip8_sdl_t *chip8_sdl);
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !CHIP8_SDL
