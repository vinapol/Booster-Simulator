#pragma once
#include "booster.hpp"

class Autopilot {
private:
    bool active = false;

public:
    bool isActive() const;
    void toggle();
    void setActive(bool act);

    void compute_controls(const Rocket& rocket, const Vector3D& wind, double gravity, double dt,
                          double& out_throttle, double& out_gimbal_x, double& out_gimbal_z);
};

