#include "draw_sky.hpp"
#include <GL/gl.h>
#include <cstdint>

void draw_sky(double rocket_y) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    double h_factor = rocket_y / 3000.0;
    if (h_factor > 1.0) h_factor = 1.0;
    if (h_factor < 0.0) h_factor = 0.0;

    uint8_t r_top = (uint8_t)(8.0 * (1.0 - h_factor));
    uint8_t g_top = (uint8_t)(10.0 * (1.0 - h_factor));
    uint8_t b_top = (uint8_t)(20.0 * (1.0 - h_factor));
    uint8_t r_bot = (uint8_t)(20.0 + 70.0 * (1.0 - h_factor));
    uint8_t g_bot = (uint8_t)(30.0 + 120.0 * (1.0 - h_factor));
    uint8_t b_bot = (uint8_t)(50.0 + 190.0 * (1.0 - h_factor));

    glBegin(GL_QUADS);
    glColor3ub(r_bot, g_bot, b_bot);
    glVertex2d(-1.0, -1.0);
    glVertex2d(1.0, -1.0);
    glColor3ub(r_top, g_top, b_top);
    glVertex2d(1.0, 1.0);
    glVertex2d(-1.0, 1.0);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}
