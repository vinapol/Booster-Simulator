#pragma once

#include "keyboardInput.hpp"
#include "joystickInput.hpp"

// Agrège clavier + joystick : le joystick prime s'il est connecté
class InputManager {
private:
    KeyboardInput keyboard;
    JoystickInput joystick;

    double throttle = 0.0;
    double tilt_x = 0.0;
    double tilt_z = 0.0;

public:
    void update(double dt);

    bool isResetPressed() const;
    bool isResetEasyPressed() const;
    bool isResetVeryEasyPressed() const;
    bool isQuitPressed() const;
    bool isAutopilotToggled() const;

    void reset();

    double getThrottle() const;
    double getTiltX() const;
    double getTiltZ() const;

    bool hasJoystick() const;
    const JoystickInput& getJoystick() const;
    JoystickInput& getJoystick();
};
