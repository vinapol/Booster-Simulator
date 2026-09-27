#pragma once

struct AudioData {
    volatile double throttle = 0.0;
    volatile bool has_fuel = true;
    volatile bool is_finished = false;
    double last_sample = 0.0;
};
