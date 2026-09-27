#include "draw_booster_and_flame_3d.hpp"
#include <GL/gl.h>
#include <cmath>
#include <cstdlib>

void draw_booster_and_flame_3d(const Rocket& myRocket, double tilt_x, double tilt_z, double throttle) {
    (void)tilt_x;
    (void)tilt_z;

    glPushMatrix();

    glTranslated(myRocket.position.x, myRocket.position.y, myRocket.position.z);

    glRotated(myRocket.theta_z * 180.0 / 3.141592653589793, 1.0, 0.0, 0.0);
    glRotated(myRocket.theta_y * 180.0 / 3.141592653589793, 0.0, 1.0, 0.0);
    glRotated(-myRocket.theta_x * 180.0 / 3.141592653589793, 0.0, 0.0, 1.0);

    int segments = 12;
    double radius = 3.0;
    double height = 30.0;

    // Fuselage
    glColor3ub(180, 185, 190);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        double angle = i * 2.0 * 3.141592653589793 / segments;
        double cx = radius * std::cos(angle);
        double cz = radius * std::sin(angle);
        glVertex3d(cx, 0.0, cz);
        glVertex3d(cx, height, cz);
    }
    glEnd();

    // Ogive
    glColor3ub(44, 62, 80);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3d(0.0, height + 4.0, 0.0);
    for (int i = 0; i <= segments; ++i) {
        double angle = i * 2.0 * 3.141592653589793 / segments;
        glVertex3d(radius * std::cos(angle), height, radius * std::sin(angle));
    }
    glEnd();

    // Anneau orange
    glColor3ub(230, 126, 34);
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        double angle = i * 2.0 * 3.141592653589793 / segments;
        double cx = radius * std::cos(angle);
        double cz = radius * std::sin(angle);
        glVertex3d(cx, height - 5.0, cz);
        glVertex3d(cx, height - 4.2, cz);
    }
    glEnd();

    // Grid fins
    glColor3ub(52, 73, 78);
    for (int j = 0; j < 4; ++j) {
        double a_fin = j * 3.141592653589793 / 2.0;
        double fx = radius * std::cos(a_fin);
        double fz = radius * std::sin(a_fin);

        glPushMatrix();
        glTranslated(fx, height - 2.1, fz);

        double defl = 0.0;
        if (j == 0) defl = myRocket.gimbal_z + myRocket.roll_cmd * 0.26;
        else if (j == 2) defl = myRocket.gimbal_z - myRocket.roll_cmd * 0.26;
        else if (j == 1) defl = myRocket.gimbal_x + myRocket.roll_cmd * 0.26;
        else if (j == 3) defl = myRocket.gimbal_x - myRocket.roll_cmd * 0.26;

        glRotated(defl * 180.0 / 3.14159265 * 1.5, 0.0, 1.0, 0.0);

        glBegin(GL_QUADS);
        glVertex3d(0.0, -0.9, 0.0);
        glVertex3d(fx * 0.5, -0.9, fz * 0.5);
        glVertex3d(fx * 0.5, 0.9, fz * 0.5);
        glVertex3d(0.0, 0.9, 0.0);
        glEnd();

        glPopMatrix();
    }

    // Tuyère + flamme
    glPushMatrix();
    glRotated(myRocket.gimbal_z * 180.0 / 3.141592653589793, 1.0, 0.0, 0.0);
    glRotated(-myRocket.gimbal_x * 180.0 / 3.141592653589793, 0.0, 0.0, 1.0);

    glColor3ub(50, 50, 50);
    double nozzle_rad = radius * 0.4;
    double nozzle_len = 2.0;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        double angle = i * 2.0 * 3.141592653589793 / segments;
        double cx = nozzle_rad * std::cos(angle);
        double cz = nozzle_rad * std::sin(angle);
        glVertex3d(cx, 0.0, cz);
        glVertex3d(cx * 1.3, -nozzle_len, cz * 1.3);
    }
    glEnd();

    if (throttle > 0.01 && myRocket.fuel_mass > 0.0) {
        double flame_len = 5.0 + 15.0 * throttle + (rand() % 100) / 100.0 * 2.0;
        double flame_rad = nozzle_rad * (0.8 + 0.6 * throttle);

        glBegin(GL_TRIANGLE_FAN);
        glColor4ub(255, 230, 100, 240);
        glVertex3d(0.0, -nozzle_len, 0.0);

        glColor4ub(231, 76, 60, 40);
        for (int i = 0; i <= segments; ++i) {
            double angle = i * 2.0 * 3.141592653589793 / segments;
            glVertex3d(flame_rad * 1.3 * std::cos(angle), -nozzle_len - flame_len, flame_rad * 1.3 * std::sin(angle));
        }
        glEnd();
    }
    glPopMatrix();

    glPopMatrix();
}

