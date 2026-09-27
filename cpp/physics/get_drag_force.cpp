#include "get_drag_force.hpp"
#include "get_air_density.hpp"
#include <cmath>

Vector3D get_drag_force(double altitude, const Vector3D& velocity, double tilt_x, double tilt_z,
                        const Vector3D& wind) {
    Vector3D rel_velocity = velocity - wind;
    double speed = rel_velocity.length();
    if (speed < 1e-3) return {0.0, 0.0, 0.0};

    double rho = get_air_density(altitude);
    Vector3D u_rel_vel = rel_velocity / speed;

    double sin_tx = std::sin(tilt_x);
    double sin_tz = std::sin(tilt_z);
    double cos_tx = std::cos(tilt_x);
    double cos_tz = std::cos(tilt_z);

    Vector3D u_booster = {sin_tx, cos_tx * cos_tz, sin_tz};
    u_booster = u_booster.normalized();

    double cos_alpha = u_booster.dot(u_rel_vel);
    if (cos_alpha > 1.0) cos_alpha = 1.0;
    if (cos_alpha < -1.0) cos_alpha = -1.0;
    double sin_alpha = std::sqrt(1.0 - cos_alpha * cos_alpha);

    const double Cd_axial = 0.5;
    const double Cd_lateral = 1.2;
    const double Area_axial = 28.27;
    const double Area_lateral = 180.0;

    double cos_a2 = cos_alpha * cos_alpha;
    double sin_a2 = sin_alpha * sin_alpha;
    double Cd = Cd_axial * cos_a2 + Cd_lateral * sin_a2;
    double Area = Area_axial * cos_a2 + Area_lateral * sin_a2;
    double drag_magnitude = 0.5 * rho * Cd * Area * speed * speed;
    return u_rel_vel * (-drag_magnitude);
}
