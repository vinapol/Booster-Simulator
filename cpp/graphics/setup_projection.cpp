#include "setup_projection.hpp"
#include <GL/gl.h>
#include <cmath>

void setup_projection(double aspect) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double fov_radians = 45.0 * 3.141592653589793 / 180.0;
    double f = 1.0 / std::tan(fov_radians / 2.0);
    double z_near = 0.5;
    double z_far = 15000.0;
    double m[16] = {
        f / aspect, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, (z_far + z_near) / (z_near - z_far), -1.0,
        0.0, 0.0, (2.0 * z_far * z_near) / (z_near - z_far), 0.0
    };
    glMultMatrixd(m);
}
