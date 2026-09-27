#include "draw_char_opengl.hpp"
#include "font8x8.hpp"
#include <GL/gl.h>

void draw_char_opengl(char c, int x, int y, int scale) {
    if (c < 32 || c > 127) return;
    const unsigned char* bitmap = font8x8_basic[c - 32];
    glBegin(GL_QUADS);
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (bitmap[row] & (1 << (7 - col))) {
                int x1 = x + col * scale;
                int y1 = y + row * scale;
                int x2 = x1 + scale;
                int y2 = y1 + scale;
                glVertex2i(x1, y1);
                glVertex2i(x2, y1);
                glVertex2i(x2, y2);
                glVertex2i(x1, y2);
            }
        }
    }
    glEnd();
}
