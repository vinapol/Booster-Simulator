#pragma once

#include <SDL2/SDL.h>
#include <string>

void draw_string(SDL_Renderer* renderer, const std::string& str, int x, int y, int scale,
                 uint8_t r, uint8_t g, uint8_t b);
