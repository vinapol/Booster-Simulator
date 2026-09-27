#include "update_controls.hpp"
#include <algorithm>
#include <vector>

void update_controls(SimState& sim, InputManager& inputs) {
    if (sim.control_mode == 1) {
        sim.autopilot.compute_controls(sim.rocket, sim.wind, sim.gravity, sim.dt,
                                       sim.controls.throttle, sim.controls.tilt_x, sim.controls.tilt_z);
        sim.controls.roll_cmd = 0.0;
    } else if (sim.control_mode == 2 && sim.ai_loaded) {
        std::vector<double> input = {
            sim.rocket.position.x / 100.0,
            sim.rocket.position.y / 1000.0,
            sim.rocket.position.z / 100.0,
            sim.rocket.velocity.x / 100.0,
            sim.rocket.velocity.y / 100.0,
            sim.rocket.velocity.z / 100.0,
            sim.rocket.theta_x,
            sim.rocket.theta_y,
            sim.rocket.theta_z,
            sim.rocket.omega_x,
            sim.rocket.omega_y,
            sim.rocket.omega_z,
            sim.rocket.fuel_mass / 4000.0,
            sim.wind.x / 20.0,
            sim.wind.z / 20.0,
            sim.rocket.position.y / 1000.0
        };

        double ap_throttle = 0.0;
        double ap_gimbal_x = 0.0;
        double ap_gimbal_z = 0.0;
        sim.autopilot.compute_controls(sim.rocket, sim.wind, sim.gravity, sim.dt,
                                       ap_throttle, ap_gimbal_x, ap_gimbal_z);

        std::vector<double> action = sim.ai_net.forward(input);
        double max_gimbal = 15.0 * 3.141592653589793 / 180.0;

        sim.controls.throttle = std::clamp(ap_throttle + 0.2 * action[0], 0.0, 1.0);
        sim.controls.tilt_x = std::clamp(ap_gimbal_x + 0.087 * action[1], -max_gimbal, max_gimbal);
        sim.controls.tilt_z = std::clamp(ap_gimbal_z + 0.087 * action[2], -max_gimbal, max_gimbal);
        sim.controls.roll_cmd = std::clamp(action[3], -1.0, 1.0);
    } else {
        sim.controls.throttle = inputs.getThrottle();
        sim.controls.tilt_x = inputs.getTiltX();
        sim.controls.tilt_z = inputs.getTiltZ();
        sim.controls.roll_cmd = 0.0;
    }
}
