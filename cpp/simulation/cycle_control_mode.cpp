#include "cycle_control_mode.hpp"

void cycle_control_mode(SimState& sim, InputManager& inputs) {
    static bool last_ap_pressed = false;
    bool ap_pressed = inputs.isAutopilotToggled();
    if (ap_pressed && !last_ap_pressed) {
        sim.control_mode = (sim.control_mode + 1) % 3;
        if (sim.control_mode == 2 && !sim.ai_loaded) {
            sim.control_mode = 0;
        }
        sim.autopilot.setActive(sim.control_mode == 1);
    }
    last_ap_pressed = ap_pressed;
}
