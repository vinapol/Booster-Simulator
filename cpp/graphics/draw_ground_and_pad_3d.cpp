#include "draw_ground_and_pad_3d.hpp"
#include <GL/gl.h>
#include <cmath>

void draw_ground_and_pad_3d() {
    glColor3ub(35, 38, 45);
    glBegin(GL_LINES);
    for (int i = -300; i <= 300; i += 20) {
        glVertex3d(i, 0.0, -300.0);
        glVertex3d(i, 0.0, 300.0);
        glVertex3d(-300.0, 0.0, i);
        glVertex3d(300.0, 0.0, i);
    }
    glEnd();

    glColor3ub(39, 174, 96);
    glBegin(GL_QUADS);
    glVertex3d(-20.0, 0.01, -20.0);
    glVertex3d(20.0, 0.01, -20.0);
    glVertex3d(20.0, 0.01, 20.0);
    glVertex3d(-20.0, 0.01, 20.0);
    glEnd();

    glColor3ub(236, 240, 241);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3d(-20.0, 0.02, -20.0);
    glVertex3d(20.0, 0.02, -20.0);
    glVertex3d(20.0, 0.02, 20.0);
    glVertex3d(-20.0, 0.02, 20.0);
    glEnd();

    glBegin(GL_LINE_LOOP);
    for (int a = 0; a < 36; ++a) {
        double rad = a * 10.0 * 3.141592653589793 / 180.0;
        glVertex3d(10.0 * std::cos(rad), 0.02, 10.0 * std::sin(rad));
    }
    glEnd();
    glLineWidth(1.0f);
}
