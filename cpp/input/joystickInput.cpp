#include "joystickInput.hpp"

JoystickInput::JoystickInput() {
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    if (SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0);
    }
}

JoystickInput::~JoystickInput() {
    if (joystick) {
        SDL_JoystickClose(joystick);
        joystick = nullptr;
    }
}

void JoystickInput::update() {
    if (joystick) {
        SDL_JoystickUpdate();
    }
}

bool JoystickInput::isConnected() const {
    return joystick != nullptr;
}

const char* JoystickInput::getName() const {
    if (joystick) {
        return SDL_JoystickName(joystick);
    }
    return "Aucun";
}

bool JoystickInput::isResetPressed() const {
    if (!joystick) return false;
    SDL_JoystickUpdate();
    return (SDL_JoystickGetButton(joystick, 1) != 0) || (SDL_JoystickGetButton(joystick, 7) != 0);
}

bool JoystickInput::isAutopilotToggled() const {
    if (!joystick) return false;
    SDL_JoystickUpdate();
    return SDL_JoystickGetButton(joystick, 3) != 0;
}

bool JoystickInput::isButtonPressed(int button) const {
    if (!joystick) return false;
    return SDL_JoystickGetButton(joystick, button) != 0;
}

int16_t JoystickInput::getRawAxis0() const {
    if (!joystick) return 0;
    SDL_JoystickUpdate();
    return SDL_JoystickGetAxis(joystick, 0);
}

int16_t JoystickInput::getRawAxis1() const {
    if (!joystick) return 0;
    SDL_JoystickUpdate();
    return SDL_JoystickGetAxis(joystick, 1);
}

int16_t JoystickInput::getRawAxis2() const {
    if (!joystick) return 0;
    SDL_JoystickUpdate();
    return SDL_JoystickGetAxis(joystick, 2);
}

double JoystickInput::getThrottle() const {
    if (!joystick) return 0.0;
    // Axe 2 : manette des gaz (T.Flight Hotas)
    int16_t val = SDL_JoystickGetAxis(joystick, 2);
    return (32767 - val) / 65535.0;
}

double JoystickInput::getTiltX() const {
    if (!joystick) return 0.0;
    int16_t val = SDL_JoystickGetAxis(joystick, 0);
    return (val / 32768.0) * (3.141592653589793 / 4.0);
}

double JoystickInput::getTiltZ() const {
    if (!joystick) return 0.0;
    int16_t val = SDL_JoystickGetAxis(joystick, 1);
    return (val / 32768.0) * (3.141592653589793 / 4.0);
}
