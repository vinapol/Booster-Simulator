#include "reset_common.hpp"

void reset_common(SimState& sim, InputManager& inputs, AudioData& audio) {
    inputs.reset();
    sim.autopilot.setActive(false);
    sim.control_mode = 2;
    sim.controls = {};
    sim.time = 0.0;
    sim.simulation_finished = false;
    sim.landed_successfully = false;
    sim.impact_speed = 0.0;
    sim.wind_noise = 0.0;
    audio.throttle = 0.0;
    audio.has_fuel = true;
    audio.is_finished = false;
}
