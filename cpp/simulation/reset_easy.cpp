#include "reset_easy.hpp"
#include "reset_common.hpp"
#include <cstdlib>

void reset_easy(SimState& sim, InputManager& inputs, AudioData& audio) {
    double random_z = ((rand() % 200) / 100.0 - 1.0) * 10.0;
    sim.rocket = Rocket(10000.0, 4000.0, 3000.0, 100.0, {15.0, 1000.0, random_z}, {0.0, -50.0, 0.0});
    reset_common(sim, inputs, audio);
    sim.wind_base = 0.0;
}
