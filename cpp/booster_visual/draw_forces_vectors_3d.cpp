#include "draw_forces_vectors_3d.hpp"
#include "get_drag_force.hpp"
#include <GL/gl.h>
#include <cmath>

void draw_forces_vectors_3d(const Rocket& myRocket, double tilt_x, double tilt_z,
                            const Vector3D& wind, double throttle, double gravity) {
    (void)tilt_x;
    (void)tilt_z;

    Vector3D com = myRocket.position + Vector3D{0.0, 15.0, 0.0};
    double scale = 0.00015;

    double mass = myRocket.dry_mass + myRocket.fuel_mass;
    double f_grav = mass * gravity;
    glColor3ub(46, 204, 113);
    glBegin(GL_LINES);
    glVertex3d(com.x, com.y, com.z);
    glVertex3d(com.x, com.y - f_grav * scale, com.z);
    glEnd();

    double thrust = myRocket.fuel_mass > 0
        ? myRocket.max_mass_flow_rate * throttle * myRocket.effective_exhaust_velocity
        : 0.0;
    double thrust_angle_x = myRocket.theta_x + myRocket.gimbal_x;
    double thrust_angle_z = myRocket.theta_z + myRocket.gimbal_z;
    double sin_ax = std::sin(thrust_angle_x);
    double sin_az = std::sin(thrust_angle_z);
    double cos_ax = std::cos(thrust_angle_x);
    double cos_az = std::cos(thrust_angle_z);
    Vector3D thrust_dir = {sin_ax, cos_ax * cos_az, sin_az};
    thrust_dir = thrust_dir.normalized();

    glColor3ub(52, 152, 219);
    glBegin(GL_LINES);
    glVertex3d(myRocket.position.x, myRocket.position.y, myRocket.position.z);
    glVertex3d(myRocket.position.x + thrust_dir.x * thrust * scale,
               myRocket.position.y + thrust_dir.y * thrust * scale,
               myRocket.position.z + thrust_dir.z * thrust * scale);
    glEnd();

    Vector3D drag = get_drag_force(myRocket.position.y, myRocket.velocity,
                                   myRocket.theta_x, myRocket.theta_z, wind);
    glColor3ub(241, 196, 15);
    glBegin(GL_LINES);
    glVertex3d(com.x, com.y, com.z);
    glVertex3d(com.x + drag.x * scale,
               com.y + drag.y * scale,
               com.z + drag.z * scale);
    glEnd();
}
