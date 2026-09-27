#include "inputManager.hpp"

void InputManager::update(double dt) {
    if (joystick.isConnected()) {
        joystick.update();
        throttle = joystick.getThrottle();
        tilt_x = joystick.getTiltX();
        tilt_z = joystick.getTiltZ();
    } else {
        keyboard.update(dt);
        throttle = keyboard.getThrottle();
        tilt_x = keyboard.getTiltX();
        tilt_z = keyboard.getTiltZ();
    }
}

bool InputManager::isResetPressed() const {
    return keyboard.isResetPressed() || joystick.isResetPressed();
}

bool InputManager::isResetEasyPressed() const {
    return keyboard.isResetEasyPressed();
}

bool InputManager::isResetVeryEasyPressed() const {
    return keyboard.isResetVeryEasyPressed();
}

bool InputManager::isQuitPressed() const {
    return keyboard.isQuitPressed();
}

bool InputManager::isAutopilotToggled() const {
    return keyboard.isAutopilotToggled() || joystick.isAutopilotToggled();
}

void InputManager::reset() {
    keyboard.reset();
    throttle = 0.0;
    tilt_x = 0.0;
    tilt_z = 0.0;
}

double InputManager::getThrottle() const { return throttle; }
double InputManager::getTiltX() const { return tilt_x; }
double InputManager::getTiltZ() const { return tilt_z; }

bool InputManager::hasJoystick() const {
    return joystick.isConnected();
}

const JoystickInput& InputManager::getJoystick() const {
    return joystick;
}

JoystickInput& InputManager::getJoystick() {
    return joystick;
}
