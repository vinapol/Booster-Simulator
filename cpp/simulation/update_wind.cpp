#include "update_wind.hpp"
#include <cmath>
#include <cstdlib>

void update_wind(SimState& sim) {
    double wind_alt_factor = 0.3 + 0.7 * (sim.rocket.position.y / 1000.0);
    if (wind_alt_factor < 0.3) wind_alt_factor = 0.3;

    if (!sim.simulation_finished) {
        sim.wind_noise = 0.98 * sim.wind_noise + 0.02 * (((rand() % 200) / 100.0 - 1.0) * 12.0);
    }

    sim.current_wind_x = (sim.wind_base * std::cos(sim.time * 0.15) + sim.wind_noise) * wind_alt_factor;
    double current_wind_z = (sim.wind_base * 0.35 * std::sin(sim.time * 0.25)) * wind_alt_factor;
    sim.wind = {sim.current_wind_x, 0.0, current_wind_z};
}
