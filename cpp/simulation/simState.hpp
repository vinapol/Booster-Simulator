#pragma once

#include "booster.hpp"
#include "inputManager.hpp"
#include "autopilot.hpp"
#include "neuralNetwork.hpp"

struct SimControls {
    double throttle = 0.0;
    double tilt_x = 0.0;
    double tilt_z = 0.0;
    double roll_cmd = 0.0;
};

struct SimState {
    Rocket rocket;
    Autopilot autopilot;
    NeuralNetwork ai_net;
    bool ai_loaded = false;
    int control_mode = 0;

    double gravity = 9.81;
    double dt = 0.016;
    double time = 0.0;

    double wind_base = 0.0;
    double wind_noise = 0.0;
    Vector3D wind = {0.0, 0.0, 0.0};
    double current_wind_x = 0.0;

    SimControls controls;
    double current_thrust = 0.0;

    bool simulation_finished = false;
    bool landed_successfully = false;
    double impact_speed = 0.0;

    // argv0 permet de retrouver weights/ même si on lance depuis builds/
    explicit SimState(const char* argv0 = nullptr);
};
