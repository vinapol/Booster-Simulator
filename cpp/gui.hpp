#ifndef GUI_HPP
#define GUI_HPP

#include <SDL2/SDL.h>
#include "booster.hpp"
#include "inputManager.hpp"

// Rendu du tableau de bord de télémétrie (Volet Droit)
void draw_telemetry(SDL_Renderer* renderer, const Rocket& myRocket, double time, double throttle, double tilt_x, double tilt_z, const Vector3D& wind, double current_wind_x, bool simulation_finished, bool landed_successfully, double impact_speed, InputManager& inputs, int control_mode);

#endif
