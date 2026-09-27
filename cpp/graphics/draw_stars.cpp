#include "draw_stars.hpp"
#include <GL/gl.h>

void draw_stars(double rocket_y, const std::vector<Star>& stars) {
    (void)rocket_y;
    glDisable(GL_LIGHTING);
    glBegin(GL_POINTS);
    for (const auto& star : stars) {
        glColor3ub(star.brightness, star.brightness, star.brightness);
        glVertex3d(star.x, star.y, star.z);
    }
    glEnd();
}
