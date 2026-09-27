#include "keyboardInput.hpp"
#include <SDL2/SDL.h>
#include <algorithm>

void KeyboardInput::update(double dt) {
    const Uint8* state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_Z]) {
        throttle += 0.6 * dt;
    }
    if (state[SDL_SCANCODE_S]) {
        throttle -= 0.6 * dt;
    }
    throttle = std::clamp(throttle, 0.0, 1.0);

    if (state[SDL_SCANCODE_Q] || state[SDL_SCANCODE_LEFT]) {
        tilt_x -= 0.6 * dt;
    }
    if (state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT]) {
        tilt_x += 0.6 * dt;
    }
    tilt_x = std::clamp(tilt_x, -3.141592653589793 / 4.0, 3.141592653589793 / 4.0);

    if (state[SDL_SCANCODE_UP]) {
        tilt_z -= 0.6 * dt;
    }
    if (state[SDL_SCANCODE_DOWN]) {
        tilt_z += 0.6 * dt;
    }
    tilt_z = std::clamp(tilt_z, -3.141592653589793 / 4.0, 3.141592653589793 / 4.0);
}

bool KeyboardInput::isResetPressed() const {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    return state[SDL_SCANCODE_R] != 0;
}

bool KeyboardInput::isResetEasyPressed() const {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    return state[SDL_SCANCODE_T] != 0;
}

bool KeyboardInput::isResetVeryEasyPressed() const {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    return state[SDL_SCANCODE_Y] != 0;
}

bool KeyboardInput::isQuitPressed() const {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    return state[SDL_SCANCODE_ESCAPE] != 0;
}

bool KeyboardInput::isAutopilotToggled() const {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    return state[SDL_SCANCODE_A] != 0;
}

void KeyboardInput::reset() {
    throttle = 0.0;
    tilt_x = 0.0;
    tilt_z = 0.0;
}

double KeyboardInput::getThrottle() const { return throttle; }
double KeyboardInput::getTiltX() const { return tilt_x; }
double KeyboardInput::getTiltZ() const { return tilt_z; }
