#include "reset_hard.hpp"
#include "reset_common.hpp"
#include <cstdlib>

void reset_hard(SimState& sim, InputManager& inputs, AudioData& audio) {
    double random_z = ((rand() % 200) / 100.0 - 1.0) * 30.0;
    sim.rocket = Rocket(10000.0, 4000.0, 3000.0, 100.0, {50.0, 3000.0, random_z}, {0.0, -150.0, 0.0});
    reset_common(sim, inputs, audio);
    sim.wind_base = ((rand() % 200) / 100.0 - 1.0) * 35.0;
}
