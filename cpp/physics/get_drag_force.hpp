#pragma once

#include "physique.hpp"

Vector3D get_drag_force(double altitude, const Vector3D& velocity, double tilt_x, double tilt_z,
                        const Vector3D& wind = {0.0, 0.0, 0.0});
