#include "setup_camera.hpp"
#include <GL/gl.h>

void setup_camera(const Rocket& rocket, double time) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(0.0, 0.0, -80.0);
    double orbit_angle = time * 0.12;
    glRotated(12.0, 1.0, 0.0, 0.0);
    glRotated(-orbit_angle * 180.0 / 3.14159265, 0.0, 1.0, 0.0);
    glTranslated(-rocket.position.x, -rocket.position.y - 15.0, -rocket.position.z);
}
