#ifndef AUTOPILOT_HPP
#define AUTOPILOT_HPP

#include "booster.hpp"
#include <cmath>
#include <algorithm>

class Autopilot {
private:
    bool active = false;

public:
    bool isActive() const {
        return active;
    }

    void toggle() {
        active = !active;
    }

    void setActive(bool act) {
        active = act;
    }

    void compute_controls(const Rocket& rocket, const Vector3D& wind, double gravity, double dt, double& out_throttle, double& out_gimbal_x, double& out_gimbal_z) {
        // --- 1. CONTRÔLE VERTICAL (POUSSÉE) ---
        double alt = rocket.position.y > 0.0 ? rocket.position.y : 0.0;
        
        // Suivi du profil de vitesse parabolique
        double target_vy = -std::sqrt(2.0 * 3.8 * alt) - 0.5;
        if (alt < 3.0) {
            target_vy = -1.0; 
        }

        double error_vy = target_vy - rocket.velocity.y; 

        double mass = rocket.dry_mass + rocket.fuel_mass;
        double max_thrust = rocket.max_mass_flow_rate * rocket.effective_exhaust_velocity;
        
        double gravity_feedforward = (mass * gravity) / max_thrust;

        double control_output = gravity_feedforward + 0.35 * error_vy;
        out_throttle = std::clamp(control_output, 0.0, 1.0);

        // --- 2. CONTRÔLE HORIZONTAL : BOUCLE CASCADE ---
        double max_theta = 25.0 * 3.141592653589793 / 180.0; // Inclinaison max autorisée (25 deg)

        // AXE X (Pitch / Tangage)
        // Boucle Externe : calcule l'attitude cible basée sur la dérive de position
        double target_vx = -0.15 * rocket.position.x;
        target_vx = std::clamp(target_vx, -25.0, 25.0);
        double error_vx = target_vx - rocket.velocity.x;
        double target_theta_x = 0.05 * error_vx; // Gain proportionnel de position
        target_theta_x = std::clamp(target_theta_x, -max_theta, max_theta);

        // Boucle Interne : calcule l'angle de tuyère (gimbal) avec correction amortie (Ajustée pour les grid fins coopératifs de V9)
        double error_theta_x = target_theta_x - rocket.theta_x;
        double target_gimbal_x = 1.2 * error_theta_x - 0.85 * rocket.omega_x;

        // AXE Z (Yaw / Lacet)
        // Boucle Externe : calcule l'attitude cible basée sur la dérive de position
        double target_vz = -0.15 * rocket.position.z;
        target_vz = std::clamp(target_vz, -25.0, 25.0);
        double error_vz = target_vz - rocket.velocity.z;
        double target_theta_z = 0.05 * error_vz; // Gain proportionnel de position
        target_theta_z = std::clamp(target_theta_z, -max_theta, max_theta);

        // Boucle Interne : calcule l'angle de tuyère (gimbal) avec correction amortie (Ajustée pour les grid fins coopératifs de V9)
        double error_theta_z = target_theta_z - rocket.theta_z;
        double target_gimbal_z = -1.2 * error_theta_z + 0.85 * rocket.omega_z;

        // Limiter le gimbal physique à +/- 15 degrés
        double max_gimbal = 15.0 * 3.141592653589793 / 180.0;
        out_gimbal_x = std::clamp(target_gimbal_x, -max_gimbal, max_gimbal);
        out_gimbal_z = std::clamp(target_gimbal_z, -max_gimbal, max_gimbal);
    }
};

#endif
