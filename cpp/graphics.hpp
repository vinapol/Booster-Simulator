#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <vector>
#include "booster.hpp"

struct Star {
    double x;
    double y;
    double z;
    int brightness;
};

// Configuration de la caméra et de la projection 3D
void setup_projection(double aspect);
void setup_camera(const Rocket& rocket, double time);

// Rendu de l'environnement de simulation 3D (Volet Gauche)
void draw_sky(double rocket_y);
void draw_stars(double rocket_y, const std::vector<Star>& stars);
void draw_wind_indicator_3d(const Vector3D& wind);
void draw_ground_and_pad_3d();
void draw_booster_and_flame_3d(const Rocket& myRocket, double tilt_x, double tilt_z, double throttle);
void draw_forces_vectors_3d(const Rocket& myRocket, double tilt_x, double tilt_z, const Vector3D& wind, double throttle, double gravity);

#endif
