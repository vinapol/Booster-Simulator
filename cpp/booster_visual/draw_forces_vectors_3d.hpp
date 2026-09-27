#pragma once

#include "booster.hpp"

void draw_forces_vectors_3d(const Rocket& rocket, double tilt_x, double tilt_z,
                            const Vector3D& wind, double throttle, double gravity);
