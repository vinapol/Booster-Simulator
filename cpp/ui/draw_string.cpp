#include "draw_string.hpp"
#include "draw_char_opengl.hpp"
#include <GL/gl.h>

void draw_string(SDL_Renderer* renderer, const std::string& str, int x, int y, int scale,
                 uint8_t r, uint8_t g, uint8_t b) {
    (void)renderer;
    glColor3ub(r, g, b);
    int current_x = x;
    int current_y = y;
    for (char c : str) {
        if (c == '\n') {
            current_y += 9 * scale;
            current_x = x;
        } else {
            draw_char_opengl(c, current_x, current_y, scale);
            current_x += 8 * scale + scale;
        }
    }
}
