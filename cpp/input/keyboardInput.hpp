#pragma once

// Entrées clavier uniquement (gaz, inclinaison, actions)
class KeyboardInput {
private:
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
};
