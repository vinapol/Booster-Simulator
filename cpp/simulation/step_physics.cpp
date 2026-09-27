#include "step_physics.hpp"
#include <cmath>

void step_physics(SimState& sim, AudioData& audio) {
    sim.current_thrust = sim.rocket.get_thrust(sim.controls.throttle);

    audio.throttle = sim.controls.throttle;
    audio.has_fuel = (sim.rocket.fuel_mass > 0.0);
    audio.is_finished = false;

    sim.rocket.update_rk4(sim.dt, sim.gravity, sim.current_thrust,
                          sim.controls.tilt_x, sim.controls.tilt_z, sim.controls.roll_cmd, sim.wind);
    sim.rocket.update_mass(sim.dt, sim.controls.throttle);
    sim.time += sim.dt;

    if (sim.rocket.position.y <= 0.0) {
        sim.rocket.position.y = 0.0;
        sim.simulation_finished = true;
        sim.impact_speed = std::abs(sim.rocket.velocity.y);
        bool inside_pad = (std::abs(sim.rocket.position.x) < 20.0 && std::abs(sim.rocket.position.z) < 20.0);
        sim.landed_successfully = (sim.impact_speed <= 2.0 && inside_pad);
        audio.throttle = 0.0;
        audio.is_finished = true;
    }
}
