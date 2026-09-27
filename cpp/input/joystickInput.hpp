#pragma once

#include <SDL2/SDL.h>

// Entrées joystick uniquement (axes gaz/inclinaison, boutons actions)
class JoystickInput {
private:
    SDL_Joystick* joystick = nullptr;

public:
    JoystickInput();
    ~JoystickInput();

    JoystickInput(const JoystickInput&) = delete;
    JoystickInput& operator=(const JoystickInput&) = delete;

    void update();

    bool isConnected() const;
    const char* getName() const;

    bool isResetPressed() const;
    bool isAutopilotToggled() const;
    bool isButtonPressed(int button) const;

    // Axes bruts (diagnostic HUD)
    int16_t getRawAxis0() const;
    int16_t getRawAxis1() const;
    int16_t getRawAxis2() const;

    // Commandes normalisées
    double getThrottle() const;
    double getTiltX() const;
    double getTiltZ() const;
};
